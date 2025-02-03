#pragma once

cbuffer CameraData : register(b0, space0)
{
    float4x4 viewMatrix;
    float4x4 inverseViewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewProjectionMatrix;
    float4 cameraPosition;
    float4 cameraResolution;
};

cbuffer DrawData : register(b1, space0)
{
    uint materialIndex;
    uint meshPoolIndex;
};

struct MaterialData
{
    uint albedoTextureIndex;
};

// TODO
// draw data will be in a structured buffer, not a cbuffer
//cbuffer DrawData : register(b2, space0)
//{
//    float4x4 modelMatrix;
//};

// Bindless textures for materials
Texture2D bindlessTextures[] : register(t, space1);
StructuredBuffer<MaterialData> materialBuffer : register(t, space2);
SamplerState linearClampSampler : register(s0, space3);
SamplerState linearRepeatSampler : register(s1, space3);

MaterialData LoadMaterialData(uint materialIndex)
{
    return materialBuffer.Load(materialIndex);
}

float4 SampleTexture(uint textureIndex, SamplerState s, float2 uv)
{
    if (textureIndex == -1)
        return float4(1, 0, 1, 1); // magenta (error color)
    
    return bindlessTextures[textureIndex].Sample(s, uv);
}

float4 SampleTextureLOD(uint textureIndex, SamplerState s, float2 uv, float lod)
{
    if (textureIndex == -1)
        return float4(1, 0, 1, 1); // magenta (error color)

    return bindlessTextures[textureIndex].SampleLevel(s, uv, lod);
}

float3 GetCameraRelativePosition(float3 position)
{
    return position - cameraPosition.xyz;
}

float3 GetAbsolutePosition(float3 position)
{
    return position + cameraPosition.xyz;
}

float3 TransformCameraRelativeWorldToView(float3 positionWS)
{
    return mul(float4(positionWS, 1.0), viewMatrix).xyz;
}

float3 TransformViewToCameraRelativeWorld(float3 positionVS)
{
    return mul(float4(positionVS, 1.0), inverseViewMatrix).xyz;
}

float4 TransformCameraRelativeWorldToHClip(float3 positionRWS)
{
    return mul(float4(positionRWS, 1.0), viewProjectionMatrix);
}

float3 TransformHClipToCameraRelativeWorld(float4 positionHClip)
{
    float3 positionNDC = float3((positionHClip.xy * cameraResolution.zw) * 2 - 1, positionHClip.z);
    positionNDC.y = -positionNDC.y;
    float4 p = mul(float4(positionNDC, 1), inverseViewProjectionMatrix);
    return p.xyz / p.w;
}

float3 TransformNDCToWorldDir(float3 positionNDC)
{
    // view matrix doens't have translation because we're using camera relative
    float4 world = mul(float4(positionNDC, 1), inverseViewProjectionMatrix);
    
    return normalize(world.xyz);
}

float4 TransformWViewToHClip(float3 positionVS)
{
    return mul(float4(positionVS, 1.0), projectionMatrix);
}

float3 GetCameraForward()
{
    return -viewMatrix[2].xyz;
}

float3 GetCameraRight()
{
    return viewMatrix[0].xyz;
}

float3 GetCameraUp()
{
    return -viewMatrix[1].xyz;
}

// https://www.shadertoy.com/view/dllSW7
uint IntegerHash(uint x)
{
    x ^= x >> 15;
    x ^= (x * x) | 1u;
    x ^= x >> 17;
    x *= 0x9E3779B9u;
    x ^= x >> 13;
    return x;
}

float3 GetRandomColor(uint seed)
{
    uint hashed = IntegerHash(seed);
    return float3(
        ((hashed >> 16) & 0xFF) / 255.0,
        ((hashed >> 8) & 0xFF) / 255.0,
        (hashed & 0xFF) / 255.0
    );
}
