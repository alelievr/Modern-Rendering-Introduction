#pragma once

struct MeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    nointerpolation float meshletIndex : TEXCOORD1;
};
