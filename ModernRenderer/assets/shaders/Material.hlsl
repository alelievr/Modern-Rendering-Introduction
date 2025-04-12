#pragma once

#include "Common.hlsl"

struct BSDFData
{
    float3 baseColor;
    float diffuseRoughness;
    float3 normalWS;
    float3 geometricNormalWS;
    float3 tangent;
    float3 biTangent;
    float specularRoughness;
    float3 specularColor;
    // TODO: aniso
};

BSDFData EvaluateMaterial(InstanceData instance, float3 geometricNormalWS, float3 geometricTangentWS, float2 uv)
{
    BSDFData bsdf = (BSDFData)0;
    MaterialData material = LoadMaterialData(instance.materialIndex);
    
    // geometric information
    bsdf.geometricNormalWS = geometricNormalWS;
    bsdf.normalWS = geometricNormalWS; // TODO: normal mapping
    bsdf.tangent = geometricTangentWS;
    bsdf.biTangent = cross(bsdf.normalWS, bsdf.tangent); // TODO: bitangent direction, check assimp tangent vector generation
    
    bsdf.baseColor = material.baseColor;
    if (material.baseColorTextureIndex != -1)
        bsdf.baseColor *= SampleTextureLOD(material.baseColorTextureIndex, linearClampSampler, uv, 0).rgb;
    
    bsdf.diffuseRoughness = material.diffuseRoughness;
    if (material.diffuseRoughnessTextureIndex != -1)
        bsdf.diffuseRoughness *= SampleTextureLOD(material.diffuseRoughnessTextureIndex, linearClampSampler, uv, 0).r;
    
    bsdf.specularRoughness = material.specularRoughness;
    bsdf.specularColor = material.specularColor;
    if (material.specularColorTextureIndex != -1)
        bsdf.specularColor *= SampleTextureLOD(material.specularColorTextureIndex, linearClampSampler, uv, 0).rgb;
    
    return bsdf;
}
