#include "Common.hlsl"
#include "MeshUtils.hlsl"

struct VisibilityMeshToFragment
{
    float4 positionCS : SV_Position;
    nointerpolation uint packedVisibilityData : TEXCOORD0;
};

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadID : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    uint groupIndex : SV_GroupIndex,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices VisibilityMeshToFragment vertices[MAX_OUTPUT_VERTICES])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    uint meshletIndex = groupID + meshletOffset;
    Meshlet meshlet = meshlets[meshletIndex];
    uint instanceID = groupIndex / meshlet.vertexCount;

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
