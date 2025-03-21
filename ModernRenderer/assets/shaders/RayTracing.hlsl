#include "Common.hlsl"
#include "GeometryUtils.hlsl"
#include "Random.hlsl"

#define MAX_PATH_DEPTH 8

RWTexture2D<float4> _Output : register(u0, space0);

RaytracingAccelerationStructure _RTAS : register(t1, space0);

struct RayPayload
{
    float3 color;
    uint intersectionCount : 4;
    uint done : 1;
    uint unused : 27;
};

[shader("raygeneration")]
void ray_gen()
{
    uint2 positionSS = DispatchRaysIndex().xy;
    float2 pixelPosition = positionSS;
    
    // Add subpixel jitter for accumulation
    uint2 temporalOffset = uint2(pathTracingFrameIndex * 29, -pathTracingFrameIndex * 31);
    pixelPosition += FloatHash22(positionSS + temporalOffset);
    
    float3 positionNDC = float3(pixelPosition * cameraResolution.zw * 2 - 1, 1);
    positionNDC.y = -positionNDC.y; // TODO: investigate why this is needed

    float3 viewDirWS = TransformNDCToWorldDir(positionNDC);
    
    RayDesc ray;
    ray.Origin = cameraPosition.xyz;
    ray.Direction = viewDirWS;
    ray.TMin = cameraNearPlane;
    ray.TMax = cameraFarPlane;

    RayPayload payload;
    TraceRay(_RTAS, 0, 0xFF, 0, 0, 0, ray, payload);
    _Output[DispatchRaysIndex().xy] += float4(payload.color, 1);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float3(0.0, 0.2, 0.4);
}