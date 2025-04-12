#pragma once

#include "MathUtils.hlsl"
#include "Material.hlsl"
#include "Random.hlsl"

#define FUJII_CONSTANT_1 (0.5 - 2.0 / (3.0 * PI))
#define FUJII_CONSTANT_2 (2.0 / 3.0 - 28.0 / (15.0 * PI))

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
    float Si = sqrt(max(0.0, 1.0 - Sq(cosTheta)));
    float G = Si * (acos(clamp(cosTheta, -1.0, 1.0)) - Si * cosTheta) +
              2.0 * ((Si / cosTheta) * (1.0 - Si * Si * Si) - Si) / 3.0;
    return A + (B * G * INV_PI);
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
    float3 colorMultiScatter = Sq(color) * avgAlbedo /
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
    float3 colorMultiScatter = Sq(color) * avgAlbedo /
                               (float3(1.0, 1.0, 1.0) - color * max(0.0, 1.0 - avgAlbedo));
    return lerp(colorMultiScatter, color, dirAlbedo);
}

// Schlick's approximation for Fresnel
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
float FresnelSchlick(float f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return (f90 - f0) * x5 + f0;
}

float FresnelSchlick(float f0, float u)
{
    return FresnelSchlick(f0, 1.0, u);
}

float3 FresnelSchlick(float3 f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return f0 * (1.0 - x5) + (f90 * x5);
}

float3 FresnelSchlick(float3 f0, float u)
{
    return FresnelSchlick(f0, 1.0, u);
}

// Ref: Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs, p. 19, 29.
// p. 84 (37/60)
float G_MaskingSmithGGX(float NdotV, float roughness)
{
    // G1(V, H)    = HeavisideStep(VdotH) / (1 + Lambda(V)).
    // Lambda(V)        = -0.5 + 0.5 * sqrt(1 + 1 / a^2).
    // a           = 1 / (roughness * tan(theta)).
    // 1 + Lambda(V)    = 0.5 + 0.5 * sqrt(1 + roughness^2 * tan^2(theta)).
    // tan^2(theta) = (1 - cos^2(theta)) / cos^2(theta) = 1 / cos^2(theta) - 1.
    // Assume that (VdotH > 0), e.i. (acos(LdotV) < Pi).

    return 1.0 / (0.5 + 0.5 * sqrt(1.0 + Sq(roughness) * (1.0 / Sq(NdotV) - 1.0)));
}

// The OpenPBR specification uses energy conpensated oren nayar lighting model for the diffuse slab
float3 EvaluateDiffuseSlab(BSDFData bsdf, float3 wo, float3 wi, float pdf)
{
    // Compute the diffuse reflection
    float NdotL = max(0.0, dot(bsdf.normalWS, wi));
    float NdotV = max(0.0, dot(bsdf.normalWS, wo));

    if (NdotL == 0.0 || NdotV == 0.0)
        return 0.0; // No contribution
    
    float3 eon = oren_nayar_compensated_diffuse(NdotV, NdotL, dot(wi, wo), bsdf.diffuseRoughness, bsdf.baseColor);
    
    // Compute the diffuse reflection
    return eon / pdf;
}

float D_GGX(float NdotH, float roughness)
{
    float a2 = Sq(roughness);
    float s = (NdotH * a2 - NdotH) * NdotH + 1.0;

    // If roughness is 0, returns (NdotH == 1 ? 1 : 0).
    // That is, it returns 1 for perfect mirror reflection, and 0 otherwise.
    return INV_PI * SafeDiv(a2, s * s);
}

float3 EvaluateGlossySlab(BSDFData bsdf, float3 wo, float3 wi, float pdf)
{
    // Compute the glossy reflection
    float3 h = normalize(wo + wi);
    float NdotL = max(0.0, dot(bsdf.normalWS, wi));
    float NdotV = max(0.0, dot(bsdf.normalWS, wo));
    float NdotH = max(0.0, dot(bsdf.normalWS, h));

    if (NdotL == 0.0 || NdotV == 0.0)
        return 0.0; // No contribution

    // Compute the GGX reflection
    float D = D_GGX(NdotH, bsdf.specularRoughness);
    float G = G_MaskingSmithGGX(NdotL, bsdf.specularRoughness);
    float3 F = FresnelSchlick(float3(0.04, 0.04, 0.04), max(0.0, dot(h, wo)));

    return (D * G * F) / (4.0 * NdotL * NdotV * pdf);
}

// https://schuttejoe.github.io/post/ggximportancesamplingpart2/
float3 GetRandomDirectionGGX_VNDF(float3 wo, float roughness, float u1, float u2)
{
    // -- Stretch the view vector so we are sampling as though
    // -- roughness==1
    float3 v = normalize(float3(wo.x * roughness, wo.y, wo.z * roughness));

    // -- Build an orthonormal basis with v, t1, and t2
    float3 t1 = (v.y < 0.999f) ? normalize(cross(v, float3(0, 1, 0))) : float3(0, 0, 1);
    float3 t2 = cross(t1, v);
    
    // -- Choose a point on a disk with each half of the disk weighted
    // -- proportionally to its projection onto direction v
    float a = 1.0f / (1.0f + v.y);
    float r = sqrt(u1);
    float phi = (u2 < a) ? (u2 / a) * PI
                         : PI + (u2 - a) / (1.0f - a) * PI;
    float p1 = r * cos(phi);
    float p2 = r * sin(phi) * ((u2 < a) ? 1.0f : v.y);

    // -- Calculate the normal in this stretched tangent space
    float3 n = p1 * t1 + p2 * t2 + sqrt(max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;

    // -- unstretch and normalize the normal
    return normalize(float3(roughness * n.x, max(0.0f, n.y), roughness * n.z));
}

float3 ComputeGlossyDirectionalReflectance(float NdotV, float roughness, float3 F0)
{
    float G = G_MaskingSmithGGX(NdotV, roughness);
    float3 Favg = 0.5 * (FresnelSchlick(F0, 0.0) + FresnelSchlick(F0, 1.0));
    return Favg * G;
}

float D_AnisoGGX(float roughnessX,
                 float roughnessY,
                 float3 H)
{
    return rcp(PI * roughnessX * roughnessY * Sq(Sq(H.x / roughnessX) + Sq(H.y / roughnessY) + Sq(H.z)));
}

float Lambda_AnisoGGX(float roughnessX,
                      float roughnessY,
                      float3 V)
{
    return 0.5 * (sqrt(1.0 + (Sq(roughnessX * V.x) + Sq(roughnessY * V.y)) / Sq(V.z)) - 1.0);
}

bool EvaluateOpenPBRSurface(BSDFData bsdf, float3 outgoingDirectionWS, out float3 incomingDirectionWS, out float3 throughput)
{
    // Transform the outgoing direction to a local tangent space
    float3x3 tangentFrame = float3x3(bsdf.tangent, bsdf.normalWS, bsdf.biTangent);
    float3x3 inverseTangentFrame = transpose(tangentFrame); // tangent frame is a rotation matrix, transpose = inverse.
    
    float3 outgoingDirectionTS = mul(outgoingDirectionWS, inverseTangentFrame);
    
    // Evaluate random for the new direction
    float u1 = FloatHash21(outgoingDirectionWS.xy + float2(pathTracingFrameIndex * 0.001, pathTracingFrameIndex * 0.0017));
    float u2 = FloatHash21(outgoingDirectionWS.yz + float2(pathTracingFrameIndex * 0.0017, pathTracingFrameIndex * 0.0023));
    
    // Determine next direction using importance sampling of roughness only to keep things simple
    float3 microfacetNormalTS = GetRandomDirectionGGX_VNDF(outgoingDirectionTS, bsdf.diffuseRoughness, u1, u2);
    
    
    float3 incomingDirectionTS = reflect(-outgoingDirectionTS, microfacetNormalTS);
    
    // Convert back to world space
    incomingDirectionWS = mul(incomingDirectionTS, tangentFrame);
    
    if (microfacetNormalTS.y < 0.001 || dot(bsdf.geometricNormalWS, incomingDirectionWS) < 0)
    {
        incomingDirectionWS = 0;
        throughput = 0.0;
        return false;
    }

    // Compute utility variables
    float3 h = normalize(outgoingDirectionWS + incomingDirectionWS);
    float NdotV = max(0.0, dot(bsdf.normalWS, outgoingDirectionWS));
    float NdotL = max(0.0, dot(bsdf.normalWS, incomingDirectionWS));
    float VdotH = max(0.0, dot(outgoingDirectionWS, h));
    float NdotH = max(0.0, dot(bsdf.normalWS, h));
    
    float pdf = 1.0; // TODO
    //float pdf = bsdf.diffuseRoughness > 0.001 ? D_GGX(NdotH, bsdf.diffuseRoughness) * NdotH / (4.0 * VdotH) : 1.0;
    
    // Compute the diffuse part of the surface, using the energy conserving Oren-Nayar BRDF in OpenPBR
    float3 diffuseSlab = EvaluateDiffuseSlab(bsdf, outgoingDirectionWS, incomingDirectionWS, pdf);
    
    // Compute the glossy part of the surface, using the Cook-Torrance microfacet model;
    float3 glossySlab = EvaluateGlossySlab(bsdf, outgoingDirectionWS, incomingDirectionWS, pdf);
    
    // The glossy BRDF is layered on top of the diffuse BRDF:
    // Compute the directional reflectance of the glossy BRDF
    // This is the part of the light that is reflected by the glossy slab
    // The rest of the light is transmitted to the diffuse slab underneeth
    // We use this factor to combine both BRDFs while conserving energy
    float3 glossyDirectionalReflectance = ComputeGlossyDirectionalReflectance(NdotV, bsdf.diffuseRoughness, bsdf.specularColor);
    
    // albedo-scaling approximation to layer glossy and diffuse surfaces
    float3 glossyDiffuse = glossySlab + diffuseSlab * (1 - glossyDirectionalReflectance);

    throughput = glossyDiffuse;
    
    return true;
}
