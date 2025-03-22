#pragma once

struct RayPayload
{
    float3 color;
    float3 worldPosition;
    float3 nextDirection;
    
    uint intersectionCount : 4;
    uint done : 1;
    uint unused : 27;
};

float3 BarycentricInterpolation(float3 v0, float3 v1, float3 v2, float2 bary)
{
    return v0 * (1 - bary.x - bary.y) + v1 * bary.x + v2 * bary.y;
}

float2 BarycentricInterpolation(float2 v0, float2 v1, float2 v2, float2 bary)
{
    return v0 * (1 - bary.x - bary.y) + v1 * bary.x + v2 * bary.y;
}
