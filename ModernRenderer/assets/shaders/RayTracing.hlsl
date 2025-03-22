#include "Common.hlsl"
#include "GeometryUtils.hlsl"
#include "Random.hlsl"
#include "PathTracingUtils.hlsl"

#define MAX_PATH_DEPTH 8

RWTexture2D<float4> _Output : register(u0, space0);

RaytracingAccelerationStructure _RTAS : register(t1, space0);

[shader("raygeneration")]
void RayGen()
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

    RayPayload payload = (RayPayload)0;
    payload.nextDirection = ray.Direction;
    
    int depth = 0;
    while (!payload.done && depth < MAX_PATH_DEPTH)
    {
        TraceRay(_RTAS, 0, 0xFF, 0, 0, 0, ray, payload);
        
        ray.Direction = payload.nextDirection;
        ray.Origin = payload.worldPosition;
        ray.TMin = 0.001;
        depth++;
    }
    
    _Output[DispatchRaysIndex().xy] += float4(payload.color, 1);
}
