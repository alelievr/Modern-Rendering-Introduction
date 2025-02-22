#include "common.hlsl"

struct MeshToFragment
{
    float4 pos: SV_POSITION;
    float4 uv: TEXCOORD0;
};

float4 main(MeshToFragment input) : SV_TARGET
{
    MaterialData material = LoadMaterialData(materialIndex);
    float2 uv = input.uv.xy;
    
    float4 albedo = SampleTextureLOD(material.baseColorTextureIndex, linearRepeatSampler, uv, 0);
    
    return albedo;
    
    //float3 positionRWS = TransformHClipToCameraRelativeWorld(input.pos);
    //float3 positionWS = GetAbsolutePosition(positionRWS);
    //float distance = length(positionRWS);
    //return float4(distance, distance / 100.0, distance / 100.0, 1);
}
