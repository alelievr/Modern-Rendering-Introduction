#include "Common.hlsl"
#include "GeometryUtils.hlsl"

RWTexture2D<float4> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint2 positionSS = threadID.xy;
    float2 outputSize;
    _Output.GetDimensions(outputSize.x, outputSize.y);
    
    float3 positionNDC = float3((positionSS / outputSize) * 2 - 1, 1);
    positionNDC.y = -positionNDC.y; // TODO: investigate why this is needed

    float3 viewDirWS = TransformNDCToWorldDir(positionNDC);

    float3 rayDir = viewDirWS;
    float3 origin = cameraPosition.xyz;
    
    float2 intersections;
    float3 spherePos = float3(0, 0, 0);
    if (IntersectRaySphere(origin - spherePos, rayDir, 1, intersections) && intersections.x > 0)
    {
        float3 positionWS = origin + rayDir * intersections.x;
        _Output[positionSS] = float4(positionWS, 1);
    }
    else
    {
        _Output[positionSS] = float4(rayDir * 0.5 + 0.5, 1);
    }
}