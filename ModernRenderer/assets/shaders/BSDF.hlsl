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
    
    // Return BRDF value (not divided by PDF - that's handled in the main function)
    return eon * INV_PI * NdotL;
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
    float VdotH = max(0.0, dot(h, wo));

    if (NdotL == 0.0 || NdotV == 0.0)
        return 0.0; // No contribution

    // Compute the GGX reflection using Cook-Torrance microfacet model
    float D = D_GGX(NdotH, bsdf.specularRoughness);
    float G = G_MaskingSmithGGX(NdotV, bsdf.specularRoughness) * G_MaskingSmithGGX(NdotL, bsdf.specularRoughness);
    float3 F = FresnelSchlick(bsdf.specularColor, VdotH);

    // Return BRDF value (not divided by PDF - that's handled in the main function)
    return (D * G * F) / (4.0 * NdotL * NdotV) * NdotL;
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
    float u3 = FloatHash21(outgoingDirectionWS.xz + float2(pathTracingFrameIndex * 0.0013, pathTracingFrameIndex * 0.0019));
    
    // Use the properly computed F0 and diffuse color from the material evaluation
    float3 f0 = bsdf.specularColor;
    float3 diffuseColor = bsdf.baseColor;
    
    // Choose between diffuse and specular sampling based on energy
    float3 specularReflectance = FresnelSchlick(f0, max(0.0, dot(bsdf.normalWS, outgoingDirectionWS)));
    float specularWeight = max(specularReflectance.r, max(specularReflectance.g, specularReflectance.b));
    float diffuseWeight = (1.0 - specularWeight) * dot(diffuseColor, float3(0.299, 0.587, 0.114)); // Luminance
    float totalWeight = specularWeight + diffuseWeight;
    
    if (totalWeight < FLOAT_EPSYLON)
    {
        incomingDirectionWS = 0;
        throughput = 0.0;
        return false;
    }
    
    float specularProbability = specularWeight / totalWeight;
    
    float3 incomingDirectionTS;
    float pdf;
    bool isSpecular = u3 < specularProbability;
    
    if (isSpecular)
    {
        // Sample specular reflection using GGX VNDF
        float3 microfacetNormalTS = GetRandomDirectionGGX_VNDF(outgoingDirectionTS, bsdf.specularRoughness, u1, u2);
        incomingDirectionTS = reflect(-outgoingDirectionTS, microfacetNormalTS);
        
        // Compute PDF for specular sampling
        float3 h = normalize(outgoingDirectionTS + incomingDirectionTS);
        float NdotH = max(0.0, h.y); // Normal is (0,1,0) in tangent space
        float VdotH = max(0.0, dot(outgoingDirectionTS, h));
        pdf = D_GGX(NdotH, bsdf.specularRoughness) * NdotH / (4.0 * VdotH) * specularProbability;
    }
    else
    {
        // Sample diffuse reflection using cosine-weighted hemisphere
        float cosTheta = sqrt(max(0.0, u1));
        float sinTheta = sqrt(max(0.0, 1.0 - u1));
        float phi = 2.0 * PI * u2;
        
        incomingDirectionTS = float3(sinTheta * cos(phi), cosTheta, sinTheta * sin(phi));
        pdf = cosTheta * INV_PI * (1.0 - specularProbability);
    }
    
    // Convert back to world space
    incomingDirectionWS = mul(incomingDirectionTS, tangentFrame);
    
    // Check for valid directions
    if (incomingDirectionTS.y < 0.001 || dot(bsdf.geometricNormalWS, incomingDirectionWS) < 0)
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
    
    if (pdf < FLOAT_EPSYLON)
    {
        incomingDirectionWS = 0;
        throughput = 0.0;
        return false;
    }
    
    // Compute the diffuse and specular components
    float3 diffuseBRDF = float3(0, 0, 0);
    float3 specularBRDF = float3(0, 0, 0);
    
    // Only evaluate diffuse if we have non-zero diffuse color (non-metallic)
    if (dot(diffuseColor, diffuseColor) > FLOAT_EPSYLON)
    {
        float3 eon = oren_nayar_compensated_diffuse(NdotV, NdotL, dot(incomingDirectionWS, outgoingDirectionWS), bsdf.diffuseRoughness, diffuseColor);
        diffuseBRDF = eon * INV_PI;
    }
    
    // Evaluate specular BRDF
    float D = D_GGX(NdotH, bsdf.specularRoughness);
    float G = G_MaskingSmithGGX(NdotV, bsdf.specularRoughness) * G_MaskingSmithGGX(NdotL, bsdf.specularRoughness);
    float3 F = FresnelSchlick(f0, VdotH);
    
    specularBRDF = (D * G * F) / (4.0 * NdotV * NdotL + FLOAT_EPSYLON);
    
    // Combine contributions according to OpenPBR layering model
    float3 brdf = diffuseBRDF + specularBRDF;
    
    // Apply cosine term and divide by PDF
    throughput = brdf * NdotL / pdf;
    
    return true;
}

// Environment lighting evaluation functions for reuse across rendering paths

// Evaluate diffuse BRDF for environment lighting (without cosine term)
float3 EvaluateDiffuseBRDF(BSDFData bsdf, float3 wo, float3 wi)
{
    float NdotL = max(0.0, dot(bsdf.normalWS, wi));
    float NdotV = max(0.0, dot(bsdf.normalWS, wo));
    
    if (NdotL <= 0.0 || NdotV <= 0.0)
        return float3(0, 0, 0);
    
    // Only evaluate diffuse if we have non-zero diffuse color (non-metallic)
    if (dot(bsdf.baseColor, bsdf.baseColor) <= FLOAT_EPSYLON)
        return float3(0, 0, 0);
    
    float3 eon = oren_nayar_compensated_diffuse(NdotV, NdotL, dot(wi, wo), bsdf.diffuseRoughness, bsdf.baseColor);
    return eon * INV_PI;
}

// Evaluate specular BRDF for environment lighting (without cosine term)
float3 EvaluateSpecularBRDF(BSDFData bsdf, float3 wo, float3 wi)
{
    float3 h = normalize(wo + wi);
    float NdotL = max(0.0, dot(bsdf.normalWS, wi));
    float NdotV = max(0.0, dot(bsdf.normalWS, wo));
    float NdotH = max(0.0, dot(bsdf.normalWS, h));
    float VdotH = max(0.0, dot(wo, h));
    
    if (NdotL <= 0.0 || NdotV <= 0.0)
        return float3(0, 0, 0);
    
    // Compute the GGX reflection using Cook-Torrance microfacet model
    float D = D_GGX(NdotH, bsdf.specularRoughness);
    float G = G_MaskingSmithGGX(NdotV, bsdf.specularRoughness) * G_MaskingSmithGGX(NdotL, bsdf.specularRoughness);
    float3 F = FresnelSchlick(bsdf.specularColor, VdotH);
    
    return (D * G * F) / (4.0 * NdotV * NdotL + FLOAT_EPSYLON);
}

// Combined BRDF evaluation for a single light direction
float3 EvaluateOpenPBRBRDF(BSDFData bsdf, float3 wo, float3 wi)
{
    float3 diffuse = EvaluateDiffuseBRDF(bsdf, wo, wi);
    float3 specular = EvaluateSpecularBRDF(bsdf, wo, wi);
    return diffuse + specular;
}

// Simplified environment reflection evaluation for rasterization
float3 EvaluateEnvironmentReflection(BSDFData bsdf, float3 viewDir, float3 reflectionColor)
{
    float NdotV = max(0.0, dot(bsdf.normalWS, viewDir));
    
    // Apply Fresnel for environment reflections
    float3 F = FresnelSchlick(bsdf.specularColor, NdotV);
    
    // Apply a simple approximation for the geometry and distribution terms
    // In a full IBL implementation, this would use precomputed BRDF integration
    float roughnessApprox = bsdf.specularRoughness;
    float envBRDFApprox = (1.0 - roughnessApprox) * 0.8 + 0.2; // Simple approximation
    
    return reflectionColor * F * envBRDFApprox;
}

// Ambient diffuse evaluation for environment lighting
float3 EvaluateAmbientDiffuse(BSDFData bsdf, float3 ambientColor)
{
    // Only apply ambient diffuse if we have non-zero diffuse color (non-metallic)
    if (dot(bsdf.baseColor, bsdf.baseColor) <= FLOAT_EPSYLON)
        return float3(0, 0, 0);
    
    return bsdf.baseColor * ambientColor;
}

// Directional environment lighting (for key light approximation)
float3 EvaluateDirectionalEnvironment(BSDFData bsdf, float3 lightDir, float3 lightColor, float3 viewDir)
{
    float NdotL = max(0.0, dot(bsdf.normalWS, lightDir));
    
    if (NdotL <= 0.0)
        return float3(0, 0, 0);
    
    // Evaluate full BRDF for directional lighting
    float3 brdf = EvaluateOpenPBRBRDF(bsdf, viewDir, lightDir);
    
    return brdf * lightColor * NdotL;
}

// Multiple directional environment sampling for better lighting quality
float3 EvaluateMultiDirectionalEnvironment(BSDFData bsdf, float3 viewDir, Texture2D<float4> envTexture, SamplerState envSampler)
{
    float3 totalRadiance = float3(0, 0, 0);
    
    // Sample multiple directions with different weights
    float3 directions[6] = {
        float3(0, 1, 0),        // Up
        float3(0, 0.3, 1),      // Front-up
        float3(1, 0.2, 0),      // Right-up
        float3(-1, 0.2, 0),     // Left-up
        float3(0, 0.3, -1),     // Back-up
        float3(0, -0.5, 0)      // Down (softer)
    };
    
    float weights[6] = { 0.4, 0.2, 0.15, 0.15, 0.1, 0.05 };
    float roughnessLevels[6] = { 0.5, 0.7, 0.8, 0.8, 0.9, 1.0 };
    
    for (int i = 0; i < 6; ++i)
    {
        float3 lightDir = normalize(directions[i]);
        float2 uv = DirectionToLatLongCoordinate(lightDir);
        float mipLevel = roughnessLevels[i] * 8.0;
        float3 lightColor = envTexture.SampleLevel(envSampler, uv, mipLevel).rgb * weights[i];
        
        totalRadiance += EvaluateDirectionalEnvironment(bsdf, lightDir, lightColor, viewDir);
    }
    
    return totalRadiance;
}
