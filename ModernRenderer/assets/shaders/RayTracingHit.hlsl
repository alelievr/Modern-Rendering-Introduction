#include "Common.hlsl"
#include "MeshUtils.hlsl"
#include "PathTracingUtils.hlsl"
#include "Random.hlsl"

struct BSDFData
{
    float3 baseColor;
    float diffuseRoughness;
    float3 normal;
    float3 geometricNormal;
    float3 tangent;
    float3 biTangent;
};

BSDFData EvaluateMaterial(InstanceData instance, float3 geometricNormalWS, float3 geometricTangentWS, float2 uv)
{
    BSDFData bsdf = (BSDFData)0;
    MaterialData material = LoadMaterialData(instance.materialIndex);
    
    // geometric information
    bsdf.geometricNormal = geometricNormalWS;
    bsdf.tangent = geometricTangentWS;
    bsdf.biTangent = cross(bsdf.normal, bsdf.tangent); // TODO: bitangent direction, check assimp tangent vector generation
    
    bsdf.baseColor = material.baseColor;
    if (material.baseColorTextureIndex != -1)
        bsdf.baseColor *= SampleTextureLOD(material.baseColorTextureIndex, linearClampSampler, uv, 0).rgb;
    
    bsdf.diffuseRoughness = material.diffuseRoughness;
    if (material.diffuseRoughnessTextureIndex != -1)
        bsdf.diffuseRoughness *= SampleTextureLOD(material.diffuseRoughnessTextureIndex, linearClampSampler, uv, 0).r;
    
    return bsdf;
}

//float3 EvaluateBSDF(BSDFData bsdf)
//{
//    float roughnessSquared = bsdf.diffuseRoughness * bsdf.diffuseRoughness;
    
//    float c = roughnessSquared / (roughnessSquared + 0.33);
//    float d = roughnessSquared / (roughnessSquared + 0.13);
//    float e = roughnessSquared / (roughnessSquared + 0.09);
    
//    float A = INV_PI * (1.0 - 0.5 * c + 0.17 * bsdf.baseColor * d);
//    float B = INV_PI * (0.45 * e);
    
//    // Oren nayar term
//    float3 orenNayar = 
    
//    // Compensation for Oren Nayar to support multi-scattering
//    float3 compensatedON = 
    
//    // diffuse term
//    float3 diffuse = orenNayar + compensatedON;
    
//    // Glossy diffuse term = spec + diffuse
//}

[shader("closesthit")]
void Hit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint rtInstanceDataIndex = InstanceID();
    RTInstanceData data = rtInstanceData[rtInstanceDataIndex];
    uint triangleID = PrimitiveIndex();
    
    MaterialData material = LoadMaterialData(data.materialIndex);
    InstanceData instance = LoadInstance(rtInstanceDataIndex);
    
    uint indexBufferOffset = data.indexBufferOffset;
    indexBufferOffset += triangleID * 3;
    
    uint i0 = indicesBuffer[indexBufferOffset + 0];
    uint i1 = indicesBuffer[indexBufferOffset + 1];
    uint i2 = indicesBuffer[indexBufferOffset + 2];
    
    VertexData v0 = vertexBuffer[i0];
    VertexData v1 = vertexBuffer[i1];
    VertexData v2 = vertexBuffer[i2];
    
    float3 normalOS = BarycentricInterpolation(v0.normal, v1.normal, v2.normal, attribs.barycentrics);
    float3 positionOS = BarycentricInterpolation(v0.positionOS, v1.positionOS, v2.positionOS, attribs.barycentrics);
    float3 tangentOS = BarycentricInterpolation(v0.tangent, v1.tangent, v2.tangent, attribs.barycentrics);
    float2 uv = BarycentricInterpolation(v0.uv, v1.uv, v2.uv, attribs.barycentrics);
    
    float3 positionWS = TransformObjectToWorld(positionOS, instance.objectToWorld);
    float3 normalWS = TransformObjectToWorldNormal(normalOS, instance.objectToWorld);
    float3 tangentWS = TransformObjectToWorldNormal(tangentOS, instance.objectToWorld);
    
    // Evaluate direct lighting
    // TODO: cast shadow rays
    // if hit, evaluate BSDF with light contribution
    // payload.totalRadiance += contributiuon * payload.throughput;
    
    // albedo
    // Apply material BSDF to throughput for next intersection (taking into account incoming light)
    
    BSDFData bsdf = EvaluateMaterial(instance, normalWS, tangentWS, uv);
    
    //float3 d = EvaluateBSDF(bsdf);
    
    payload.throughput *= bsdf.baseColor; // TODO: read material albedo
    
    // Compute an offset for the ray origin to avoid self-intersections when the normal is not aligned with the geometric normal.
    float3 triangleNormal = normalize(cross(v1.positionOS - v0.positionOS, v2.positionOS - v0.positionOS));
    float3 originOffset = triangleNormal * (1 - dot(triangleNormal, normalWS)) * 0.0001;
    
    // Lambertian reflection
    payload.nextDirection = normalize(normalWS + RandomHemisphereVector(positionWS, normalWS));
    payload.worldPosition = positionWS + originOffset;
    payload.done = false;
}
