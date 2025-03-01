#include "Common.hlsl"
#include "MeshUtils.hlsl"

#define TASK_THREAD_GROUP_SIZE 32

struct VisibilityMeshToFragment
{
    float4 positionCS : SV_Position;
    nointerpolation uint packedVisibilityData : TEXCOORD0;
};

// TODO: amplification shader to do cluster culling

struct TaskCullingPayload
{
    uint meshletIndex[TASK_THREAD_GROUP_SIZE];
};

groupshared TaskCullingPayload cullingPayload;

[NumThreads(TASK_THREAD_GROUP_SIZE, 1, 1)]
void task(uint threadID : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
{
    // TODO: Perform frustum culling on meshlets
    uint instanceID = instanceOffset; // Instance offset is provided as extra data from the indirect dispatch
    InstanceData instance = instanceData[instanceID];
    bool visible = false;
    
    uint meshletIndex = threadID + instance.meshletIndex; // TODO: replace the meshlet index from instance by cullingPayload data
    //Meshlet meshlet = meshlets[meshletIndex];
    Bounds bounds = LoadMeshletBounds(meshletIndex, true);
    
    //if (dot(normalize(cone_apex - camera_position), cone_axis) >= cone_cutoff)
    //    reject();

    // Perform frustum culling on the meshlet
    if (SphereFrustumIntersection(cameraCullingFrustum, bounds.center, bounds.radius) > 0)
        visible = true;
    
    visible |= cameraMeshletFrustumCullingDisabled;
    
    if (visible)
    {
        uint packedIndex = WavePrefixCountBits(visible);
        cullingPayload.meshletIndex[packedIndex] = meshletIndex;
    }
    
    bool isFirstThreadOfGroup = groupThreadId == 0;
    uint visbleMeshletCount = WaveActiveCountBits(visible) * isFirstThreadOfGroup;
    DispatchMesh(visbleMeshletCount, 1, 1, cullingPayload);
}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadID : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    in payload TaskCullingPayload cullingPayload,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices VisibilityMeshToFragment vertices[MAX_OUTPUT_VERTICES])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    uint instanceID = instanceOffset; // Instance offset is provided as extra data from the indirect dispatch
    InstanceData instance = instanceData[instanceID];
    //uint meshletIndex = groupID + instance.meshletIndex; // TODO: replace the meshlet index from instance by cullingPayload data
    uint meshletIndex = cullingPayload.meshletIndex[groupID];
    Meshlet meshlet = meshlets[meshletIndex];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    for (uint i = 0; i < 2; ++i)
    {
        const uint primitiveId = threadID + i * 128;
        if (primitiveId < meshlet.triangleCount)
        {
            triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
        }
    }
    
    if (threadID < meshlet.vertexCount)
    {
        uint vertexIndex = meshletIndices[meshlet.vertexoffset + threadID];
        MeshToFragment vertex = LoadVertexAttributes(groupID, vertexIndex, instanceID);
        VisibilityMeshToFragment vout;
        
        vout.positionCS = vertex.positionCS;
        vout.packedVisibilityData = EncodeVisibility(materialIndex, meshletIndex, threadID);

        vertices[threadID] = vout;
    }
}

uint fragment(VisibilityMeshToFragment input) : SV_TARGET0
{
    return input.packedVisibilityData;
}
