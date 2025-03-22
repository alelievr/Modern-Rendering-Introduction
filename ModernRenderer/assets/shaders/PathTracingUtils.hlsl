#pragma once

struct RayPayload
{
    float3 throughput;
    float3 totalRadiance;
    float3 worldPosition;
    float3 nextDirection;
    
    uint intersectionCount : 4;
    uint done : 1;
    uint unused : 27;
};

float3 BarycentricInterpolation(float3 v0, float3 v1, float3 v2, float2 bary)
{
    float3 edge0 = v1 - v0;
    float3 edge1 = v2 - v0;
    
    return v0 + bary.x * edge0 + bary.y * edge1;
}

float2 BarycentricInterpolation(float2 v0, float2 v1, float2 v2, float2 bary)
{
    float2 edge0 = v1 - v0;
    float2 edge1 = v2 - v0;
    
    return v0 + bary.x * edge0 + bary.y * edge1;
}
