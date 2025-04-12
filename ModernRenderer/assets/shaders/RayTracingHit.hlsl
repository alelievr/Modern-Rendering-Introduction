#include "Common.hlsl"
#include "MeshUtils.hlsl"
#include "PathTracingUtils.hlsl"
#include "Random.hlsl"
#include "Material.hlsl"
#include "BSDF.hlsl"

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
    
    float3 throughput, nextDirection;
    bool sampleBSDFFailed = EvaluateOpenPBRSurface(bsdf, -WorldRayDirection(), nextDirection, throughput);
    
    // Compute an offset for the ray origin to avoid self-intersections when the normal is not aligned with the geometric normal.
    float3 triangleNormal = normalize(cross(v1.positionOS - v0.positionOS, v2.positionOS - v0.positionOS));
    // TODO: flip offset for transmitted rays
    float3 originOffset = triangleNormal * (1 - dot(triangleNormal, normalWS)) * 0.0001;

    // Output payload data
    payload.throughput *= throughput;
    payload.nextDirection = nextDirection;
    payload.worldPosition = positionWS + originOffset;
    payload.done = false;

    // Debug
    //payload.totalRadiance = nextDirection * 0.5 + 0.5;
    //payload.done = true;
}
