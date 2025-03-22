#include "Common.hlsl"
#include "MeshUtils.hlsl"
#include "PathTracingUtils.hlsl"

Texture2D<float4> _SkyTextureLatLong : register(t0, space3);

[shader("miss")]
void Miss(inout RayPayload payload)
{
    float2 uv = DirectionToLatLongCoordinate(-normalize(WorldRayDirection()));
    
    float4 skyColor = _SkyTextureLatLong.SampleLevel(linearRepeatSampler, uv, 0);
    
    payload.color = skyColor.rgb;
    payload.done = true;
}
