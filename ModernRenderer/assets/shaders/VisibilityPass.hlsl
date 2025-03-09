#include "Common.hlsl"
#include "MeshUtils.hlsl"

#define TASK_THREAD_GROUP_SIZE 32

struct VisibilityMeshToFragment
{
    float4 positionCS : SV_Position;
    nointerpolation uint packedVisibilityData : TEXCOORD0;
};

//struct TaskCullingPayload
//{
//    uint meshletIndex[TASK_THREAD_GROUP_SIZE];
//};

//groupshared TaskCullingPayload cullingPayload;

//[NumThreads(TASK_THREAD_GROUP_SIZE, 1, 1)]
//void task(uint threadID : SV_DispatchThreadID, uint groupThreadId : SV_GroupThreadID)
//{
//    // TODO: Perform frustum culling on meshlets
//    uint instanceID = instanceOffset; // Instance offset is provided as extra data from the indirect dispatch
//    InstanceData instance = instanceData[instanceID];
//    bool visible = true;
    
//    uint meshletIndex = threadID + instance.meshletIndex; // TODO: replace the meshlet index from instance by cullingPayload data
//    //Meshlet meshlet = meshlets[meshletIndex];
//    Bounds bounds = LoadMeshletBounds(meshletIndex, true);
    
//    // TODO: transform bounds to world space
//    bounds.center = TransformObjectToWorld(GetCameraRelativePosition(bounds.center), instance.objectToWorld);
    
//    // Perform cone culling to eliminate backfacing meshlets
//    if (!cameraMeshletBackfaceCullingDisabled)
//        if (dot(normalize(bounds.coneApex - cameraCullingPosition.xyz), bounds.coneAxis) >= bounds.coneCutoff)
//            visible = false;

//    // Perform frustum culling on the meshlet
//    if (!cameraMeshletFrustumCullingDisabled)
//        if (visible && SphereFrustumIntersection(cameraCullingFrustum, bounds.center, bounds.radius) <= 0)
//            visible = false;
    
//    if (visible)
//    {
//        uint packedIndex = WavePrefixCountBits(visible);
//        cullingPayload.meshletIndex[packedIndex] = meshletIndex;
//    }
    
//    bool isFirstThreadOfGroup = groupThreadId == 0;
//    uint visbleMeshletCount = WaveActiveCountBits(visible) * isFirstThreadOfGroup;
//    DispatchMesh(visbleMeshletCount, 1, 1, cullingPayload);
//}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadID : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    //in payload TaskCullingPayload cullingPayload,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices VisibilityMeshToFragment vertices[MAX_OUTPUT_VERTICES])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    VisibleMeshlet visibleMeshlet = visibleMeshlets1[groupID]; // TODO: multiple dispatch support
    uint instanceID = visibleMeshlet.instanceIndex;
    uint meshletIndex = visibleMeshlet.meshletIndex;
    
    InstanceData instance = instanceData[instanceID];
    Meshlet meshlet = meshlets[meshletIndex];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    const uint primitiveId = threadID;
    if (primitiveId < meshlet.triangleCount)
    {
        triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
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
