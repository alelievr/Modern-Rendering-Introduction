#include "Common.hlsl"
#include "MeshUtils.hlsl"

Texture2D<uint> _VisibilityTexture : register(t0, space4);

// TODO
struct VisibilityMeshToFragment
{
    float4 positionCS : SV_Position;
    // UV?
};

[NumThreads(1, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadId : SV_GroupThreadID,
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
        const uint primitiveId = threadId + i * 128;
        if (primitiveId < meshlet.triangleCount)
        {
            triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
        }
    }
    
    if (threadId < meshlet.vertexCount)
    {
        uint vertexIndex = meshletIndices[meshlet.vertexoffset + threadId];
        MeshToFragment vertex = LoadVertexAttributes(groupID, vertexIndex, instanceID);
        VisibilityMeshToFragment vout;
        
        vout.positionCS = vertex.positionCS;

        vertices[threadId] = vout;
    }
}

float4 fragment(VisibilityMeshToFragment input) : SV_TARGET0
{
    uint visibilityData = _VisibilityTexture.Load(uint3(input.positionCS.xy, 0));
    
    uint materialID, meshletID, triangleID;
    DecodeVisibility(visibilityData, materialID, meshletID, triangleID);
    
    return float4(triangleID / 255.0, 0, 1, 1);
}
