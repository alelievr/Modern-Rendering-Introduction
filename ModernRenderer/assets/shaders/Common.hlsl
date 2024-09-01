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
};

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
    return mul(viewMatrix, float4(positionWS, 1.0)).xyz;
}

float3 TransformViewToCameraRelativeWorld(float3 positionVS)
{
    return mul(inverseViewMatrix, float4(positionVS, 1.0)).xyz;
}

float4 TransformCameraRelativeWorldToHClip(float3 positionWS)
{
    return mul(viewProjectionMatrix, float4(positionWS, 1.0));
}

float3 TransformHClipToCameraRelativeWorld(float3 positionWS)
{
    return mul(inverseViewProjectionMatrix, float4(positionWS, 1.0)).xyz;
}

float3 TransformHClipToWorldDir(float3 direction)
{
    // view matrix doens't have translation because we're using camera relative
    float4 world = mul(inverseViewProjectionMatrix, float4(direction, 1.0));
    
    return normalize(world.xyz / world.w);
}

float4 TransformWViewToHClip(float3 positionVS)
{
    return mul(projectionMatrix, float4(positionVS, 1.0));
}
