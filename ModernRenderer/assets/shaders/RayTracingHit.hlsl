#include "Common.hlsl"
#include "MeshUtils.hlsl"
#include "PathTracingUtils.hlsl"

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
    
    float3 normal = BarycentricInterpolation(v0.normal, v1.normal, v2.normal, attribs.barycentrics);
    float3 positionOS = BarycentricInterpolation(v0.positionOS, v1.positionOS, v2.positionOS, attribs.barycentrics);
    float3 positionWS = TransformObjectToWorld(positionOS, instance.objectToWorld);
    
    normal = normalize(normal);
    
    float2 bary = attribs.barycentrics;
    payload.color = float3(bary, 1 - bary.x - bary.y);
    payload.color = normal * 0.5 + 0.5;
    //payload.color = material.diffuseRoughness;
    
    payload.nextDirection = reflect(WorldRayDirection(), normal);
    payload.worldPosition = positionWS;
    payload.done = false;
}
