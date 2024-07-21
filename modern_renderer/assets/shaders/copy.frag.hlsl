// #include "tone_map.hlsl"

struct VertexToFragment
{
    float4 positionCS : SV_POSITION;
};

// Uniform buffer equivalent to the GLSL uniform block
[[vk::binding(0, 0)]]
cbuffer CopyData : register(b0)
{
    float exposure_scale;
    float3x3 rec709_from_xyz;
    float3x3 acescg_from_xyz;
    uint tone_map_method;
};

[[vk::binding(1, 0)]]
[[vk::image_format("r32f")]]
RWTexture2D<float> g_result[3];

// Main entry point for the pixel shader
[[vk::location(0)]]
float4 frag(VertexToFragment i) : SV_Target
{
    // Load the result from the texture
    float4 result = g_result[0].Load(int3(i.positionCS.xy, 0));

    // Calculate the color
    float3 col = max(result.xyz * (exposure_scale / result.w), 0.0);

    // TODO: implement tone mapping
    // col = tone_map_sample(col, rec709_from_xyz, acescg_from_xyz, tone_map_method);

    // Set the output color
    return float4(col, 1.0f);
}
