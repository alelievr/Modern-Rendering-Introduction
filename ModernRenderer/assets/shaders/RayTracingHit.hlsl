#include "Common.hlsl"
#include "MeshUtils.hlsl"

struct RayPayload
{
    float3 color;
};

[shader("closesthit")]
void Hit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint indexBufferIndex = InstanceID();
    uint triangleID = PrimitiveIndex();
    
    indexBufferIndex += triangleID * 3;
    
    
    
    uint i0 = indicesBuffer[indexBufferIndex + 0];
    uint i1 = indicesBuffer[indexBufferIndex + 1];
    uint i2 = indicesBuffer[indexBufferIndex + 2];
    
    VertexData v0 = vertexBuffer[i0];
    VertexData v1 = vertexBuffer[i1];
    VertexData v2 = vertexBuffer[i2];
    
    float3 normal = BarycentricInterpolation(v0.normal, v1.normal, v2.normal, attribs.barycentrics);
    
    float2 bary = attribs.barycentrics;
    payload.color = float3(bary, 1 - bary.x - bary.y);
    payload.color = normal * 0.5 + 0.5;
}

[shader("closesthit")]
void closest_green(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    CallShader(0, payload);
}