#include "Common.hlsl"

RWTexture2D<float4> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 outputSize;
    _Output.GetDimensions(outputSize.x, outputSize.y);
    
    float3 clipPosition = float3((threadID.xy / outputSize) * 2 - 1, 1);
    float3 viewDirWS = TransformHClipToWorldDir(clipPosition);
    _Output[threadID.xy] = float4(viewDirWS * 0.5 + 0.5, 1);
}