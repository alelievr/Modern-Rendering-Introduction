#include "Common.hlsl"
#include "MeshUtils.hlsl"

float4 main(MeshToFragment input) : SV_TARGET
{
    MaterialData material = LoadMaterialData(materialIndex);
    float2 uv = input.uv.xy;
    
    float4 albedo = SampleTextureLOD(material.albedoTextureIndex, linearRepeatSampler, uv, 0);
    
    float3 positionRWS = TransformHClipToCameraRelativeWorld(input.positionCS);
    float3 dir = -normalize(positionRWS);
    
    float shading = dot(dir, input.normal);
    shading = shading * 0.8 + 0.2;
    
    albedo.rgb = GetRandomColor(input.meshletIndex) * shading;
    
    return albedo;
    
    //float3 positionRWS = TransformHClipToCameraRelativeWorld(input.pos);
    //float3 positionWS = GetAbsolutePosition(positionRWS);
    //float distance = length(positionRWS);
    //return float4(distance, distance / 100.0, distance / 100.0, 1);
}
