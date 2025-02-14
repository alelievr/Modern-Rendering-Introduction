#include "Common.hlsl"

TextureCube<float4> _SkyTexture : register(t0, space4);

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
{
    SetMeshOutputCounts(3, 1);
    
    triangles[0] = uint3(0, 1, 2);
    vertices[0] = GetFullscreenTriangleVertex(0);
    vertices[1] = GetFullscreenTriangleVertex(1);
    vertices[2] = GetFullscreenTriangleVertex(2);
}

float4 fragment(MeshToFragment input) : SV_TARGET
{
    MaterialData material = LoadMaterialData(materialIndex);
    float3 positionRWS = TransformHClipToCameraRelativeWorld(input.positionCS);
    float3 dir = -normalize(positionRWS);
    
    float4 skyColor = _SkyTexture.SampleLevel(linearRepeatSampler, dir, 0);
    
    return skyColor;
}
