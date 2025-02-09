#include "Common.hlsl"
#include "GeometryUtils.hlsl"

RWTexture2D<float4> _Output : register(u0, space0);

RaytracingAccelerationStructure _RTAS : register(t1, space0);

struct RayPayload
{
    float3 color;
};

[shader("raygeneration")]
void ray_gen()
{
    float2 outputSize;
    _Output.GetDimensions(outputSize.x, outputSize.y);
    uint2 positionSS = DispatchRaysIndex().xy;

    float3 positionNDC = float3((positionSS / outputSize) * 2 - 1, 1);
    positionNDC.y = -positionNDC.y; // TODO: investigate why this is needed

    float3 viewDirWS = TransformNDCToWorldDir(positionNDC);
    
    RayDesc ray;
    ray.Origin = cameraPosition.xyz;
    ray.Direction = viewDirWS;
    ray.TMin = 0.001; // TODO: fill with camera params
    ray.TMax = 10000.0;

    RayPayload payload;
    TraceRay(_RTAS, 0, 0xFF, 0, 0, 0, ray, payload);
    _Output[DispatchRaysIndex().xy] = float4(payload.color, 1);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float3(0.0, 0.2, 0.4);
}