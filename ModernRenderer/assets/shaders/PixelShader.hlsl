#include "common.hlsl"

struct VS_OUTPUT
{
    float4 pos: SV_POSITION;
};

cbuffer Settings : register(b1, space0)
{
    float4 color;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 positionRWS = TransformHClipToCameraRelativeWorld(input.pos);
    float3 positionWS = GetAbsolutePosition(positionRWS);
    float distance = length(positionRWS);
    return float4(distance, distance / 100.0, distance / 100.0, 1);
}
