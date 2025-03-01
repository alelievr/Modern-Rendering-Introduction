#include "Common.hlsl"

Texture2D<uint> _VisibilityTexture : register(t1, space2);

struct MeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

MeshToFragment GetFullscreenTriangleVertex(uint id)
{
    MeshToFragment v;
    
    v.positionCS = GetFullScreenTriangleVertexPosition(id);
    v.uv = GetFullScreenTriangleTexCoord(id);

    return v;
}

[NumThreads(1, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadId : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    uint groupIndex : SV_GroupIndex,
    out indices uint3 triangles[1],
    out vertices MeshToFragment vertices[3])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    SetMeshOutputCounts(3, 1);
    
    triangles[0] = uint3(0, 1, 2);
    vertices[0] = GetFullscreenTriangleVertex(0);
    vertices[1] = GetFullscreenTriangleVertex(1);
    vertices[2] = GetFullscreenTriangleVertex(2);
}

float4 fragment(MeshToFragment input) : SV_TARGET0
{
    uint visibilityData = _VisibilityTexture.Load(uint3(input.positionCS.xy, 0));
    
    uint materialID, meshletID, triangleID;
    DecodeVisibility(visibilityData, materialID, meshletID, triangleID);
    
    return float4(GetRandomColor(meshletID), 1);
}
