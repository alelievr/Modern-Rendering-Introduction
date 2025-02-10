#pragma once

struct MeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    nointerpolation float meshletIndex : TEXCOORD2;
};
