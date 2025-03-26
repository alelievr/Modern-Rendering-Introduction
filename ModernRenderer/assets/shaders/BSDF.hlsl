#pragma once

#include "MathUtils.hlsl"

const float FUJII_CONSTANT_1 = 0.5 - 2.0 / (3.0 * PI);
const float FUJII_CONSTANT_2 = 2.0 / 3.0 - 28.0 / (15.0 * PI);

float oren_nayar_fujii_diffuse_avg_albedo(float roughness)
{
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    return A * (1.0 + FUJII_CONSTANT_2 * roughness);
}

// Improved Oren-Nayar diffuse from Fujii:
// https://mimosa-pudica.net/improved-oren-nayar.html
float oren_nayar_fujii_diffuse_dir_albedo(float cosTheta, float roughness)
{
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    float B = roughness * A;
    float Si = sqrt(max(0.0, 1.0 - sq(cosTheta)));
    float G = Si * (acos(clamp(cosTheta, -1.0, 1.0)) - Si * cosTheta) +
              2.0 * ((Si / cosTheta) * (1.0 - Si * Si * Si) - Si) / 3.0;
    return A + (B * G * INV_PI);
}

float oren_nayar_fujii_diffuse_avg_albedo(float roughness)
{
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    return A * (1.0 + FUJII_CONSTANT_2 * roughness);
}

// Energy-compensated Oren-Nayar diffuse from OpenPBR Surface:
// https://academysoftwarefoundation.github.io/OpenPBR/
float3 oren_nayar_compensated_diffuse(float NdotV, float NdotL, float LdotV, float roughness, float3 color)
{
    float s = LdotV - NdotL * NdotV;
    float stinv = (s > 0.0) ? s / max(NdotL, NdotV) : s;

    // Compute the single-scatter lobe.
    float A = 1.0 / (1.0 + FUJII_CONSTANT_1 * roughness);
    float3 lobeSingleScatter = color * A * (1.0 + roughness * stinv);

    // Compute the multi-scatter lobe.
    float dirAlbedoV = oren_nayar_fujii_diffuse_dir_albedo(NdotV, roughness);
    float dirAlbedoL = oren_nayar_fujii_diffuse_dir_albedo(NdotL, roughness);
    float avgAlbedo = oren_nayar_fujii_diffuse_avg_albedo(roughness);
    float3 colorMultiScatter = sq(color) * avgAlbedo /
                               (float3(1.0, 1.0, 1.0) - color * max(0.0, 1.0 - avgAlbedo));
    float3 lobeMultiScatter = colorMultiScatter *
                              max(FLOAT_EPSYLON, 1.0 - dirAlbedoV) *
                              max(FLOAT_EPSYLON, 1.0 - dirAlbedoL) /
                              max(FLOAT_EPSYLON, 1.0 - avgAlbedo);

    // Return the sum.
    return lobeSingleScatter + lobeMultiScatter;
}

float3 oren_nayar_compensated_diffuse_dir_albedo(float cosTheta, float roughness, float3 color)
{
    float dirAlbedo = oren_nayar_fujii_diffuse_dir_albedo(cosTheta, roughness);
    float avgAlbedo = oren_nayar_fujii_diffuse_avg_albedo(roughness);
    float3 colorMultiScatter = sq(color) * avgAlbedo /
                               (float3(1.0, 1.0, 1.0) - color * max(0.0, 1.0 - avgAlbedo));
    return lerp(colorMultiScatter, color, dirAlbedo);
}

void oren_nayar_diffuse_bsdf_reflection(
    float3 L, float3 V, float3 P, float occlusion, float weight, float3 color, float roughness, float3 normal, inout BSDF bsdf)
{
    bsdf.throughput = float3(0.0, 0.0, 0.0);

    if (weight < FLOAT_EPSYLON)
        return;

    //normal = forward_facing_normal(normal, V);

    float NdotV = clamp(dot(normal, V), FLOAT_EPSYLON, 1.0);
    float NdotL = clamp(dot(normal, L), FLOAT_EPSYLON, 1.0);
    float LdotV = clamp(dot(L, V), FLOAT_EPSYLON, 1.0);

    float3 diffuse = oren_nayar_compensated_diffuse(NdotV, NdotL, LdotV, roughness, color);

    bsdf.response = diffuse * occlusion * weight * NdotL * M_PI_INV;
}
