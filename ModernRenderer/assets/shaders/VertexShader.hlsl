#include "common.hlsl"

struct VertexAttributes
{
    float3 pos : POSITION;
};

struct VertexToFragment
{
    float4 pos : SV_POSITION;
    float4 debug : TEXCOORD0;
};

VertexToFragment main(VertexAttributes input)
{
    VertexToFragment output;
    
    float3 pos = GetCameraRelativePosition(input.pos);
    
    // Assume that position is world space for now
    output.pos = TransformCameraRelativeWorldToHClip(pos);
    output.debug = float4(pos, 1);
    //output.pos = float4(pos, 1);
    //output.pos = float4(input.pos, 1);
    
    return output;
}
