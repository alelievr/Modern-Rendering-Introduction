#include "Common.hlsl"

Texture2D<float4> _PathTracingAccumulationTexture : register(t0, space0);
RWTexture2D<float4> _MainCameraColorTexture : register(u0, space0);

[numthreads(8, 8, 1)]
void clear(uint3 threadID : SV_DispatchThreadID)
{
    _MainCameraColorTexture[threadID.xy] = 0;
}

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    _MainCameraColorTexture[threadID.xy] = _PathTracingAccumulationTexture[threadID.xy] / _PathTracingAccumulationTexture[threadID.xy].w;
}
