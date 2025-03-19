#include "Common.hlsl"
#include "MeshUtils.hlsl"

struct RayPayload
{
    float3 color;
};

[shader("closesthit")]
void Hit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint rtInstanceDataIndex = InstanceID();
    RTInstanceData data = rtInstanceData[rtInstanceDataIndex];
    uint triangleID = PrimitiveIndex();
    
    MaterialData material = LoadMaterialData(data.materialIndex);
    
    uint indexBufferOffset = data.indexBufferOffset;
    indexBufferOffset += triangleID * 3;
    
    uint i0 = indicesBuffer[indexBufferOffset + 0];
    uint i1 = indicesBuffer[indexBufferOffset + 1];
    uint i2 = indicesBuffer[indexBufferOffset + 2];
    
    VertexData v0 = vertexBuffer[i0];
    VertexData v1 = vertexBuffer[i1];
    VertexData v2 = vertexBuffer[i2];
    
    float3 normal = BarycentricInterpolation(v0.normal, v1.normal, v2.normal, attribs.barycentrics);
    
    float2 bary = attribs.barycentrics;
    payload.color = float3(bary, 1 - bary.x - bary.y);
    payload.color = normal * 0.5 + 0.5;
    payload.color = material.diffuseRoughness;
}

[shader("closesthit")]
void closest_green(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    CallShader(0, payload);
}