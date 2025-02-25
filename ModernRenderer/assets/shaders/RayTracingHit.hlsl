#include "Common.hlsl"

struct VertexData
{
    float3 positionOS;
    float3 normal;
    float2 uv;
    float3 tangent;
};

// Bindless buffer of vertex data
StructuredBuffer<VertexData> vertexBuffers[] : register(t, space4);
Buffer<uint> indexBuffers[] : register(t, space5);

struct RayPayload
{
    float3 color;
};

[shader("closesthit")]
void Hit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint instanceID = InstanceID();
    uint triangleID = PrimitiveIndex();
    
    uint i0 = indexBuffers[instanceID][triangleID];
    uint i1 = indexBuffers[instanceID][triangleID + 1];
    uint i2 = indexBuffers[instanceID][triangleID + 2];
    
    VertexData v0 = vertexBuffers[instanceID][i0];
    VertexData v1 = vertexBuffers[instanceID][i1];
    VertexData v2 = vertexBuffers[instanceID][i2];
    
    float3 normal = BarycentricInterpolation(v0.normal, v1.normal, v2.normal, attribs.barycentrics);
    
    float2 bary = attribs.barycentrics;
    payload.color = float3(bary, 0);
}

[shader("closesthit")]
void closest_green(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    CallShader(0, payload);
}