#include "Common.hlsl"
#include "GeometryUtils.hlsl"

RWTexture2D<float4> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 outputSize;
    _Output.GetDimensions(outputSize.x, outputSize.y);
    
    float4 clipPosition = float4((threadID.xy / outputSize) * 2 - 1, 1, 1);
    clipPosition.y = -clipPosition.y; // TODO: investigate why this is needed

    float3 viewDirWS = TransformHClipToWorldDir(clipPosition);
    _Output[threadID.xy] = float4(viewDirWS * 0.5 + 0.5, 1);

    float aspect = outputSize.y / outputSize.x;
    //float3 rayDir = normalize(float3(clipPosition.x, clipPosition.y * aspect, -1));
    //float3 rayDir = clipPosition.x * GetCameraRight() + clipPosition.y * abs(aspect) * GetCameraUp() - GetCameraForward();
    float3 rayDir = viewDirWS;
    float3 origin = cameraPosition.xyz;
    
    //rayDir = mul(clipPosition, (float3x3)(inverseViewMatrix));
    //rayDir = normalize(rayDir);
    
    //rayDir = normalize(-viewDirWS);
    
    //TransformHClipToWorldDir(clipPosition);
    
    float2 intersections;
    float3 spherePos = float3(0, 0, 0);
    if (IntersectRaySphere(origin - spherePos, rayDir, 1, intersections) && intersections.x > 0)
    {
        float3 positionWS = origin + rayDir * intersections.x;
        _Output[threadID.xy] = float4(positionWS, 1);
    }
    else
    {
        _Output[threadID.xy] = float4(rayDir * 0.5 + 0.5, 1);
    }
}