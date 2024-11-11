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
// TODO
//Texture2D<float4> normalTextures[] : register(t, space1);
//Texture2D<float4> roughnessTextures[] : register(t, space1);

// Common samplers for textures
SamplerState PointClampSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState LinearWrapSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

MaterialData LoadMaterialData(uint materialIndex)
{
    return materialBuffer.Load(materialIndex);
}

float4 SampleTexture(uint textureIndex, SamplerState s, float2 uv)
{
    return bindlessTextures[textureIndex].Load(uint3(uv * 4096, 0));
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