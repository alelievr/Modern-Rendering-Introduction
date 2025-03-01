#pragma once

#include "GeometryUtils.hlsl"

#define PI          3.14159265358979323846
#define INV_PI      0.31830988618379067154
#define HALF_PI     1.57079632679489661923
#define LOG2_E      1.44269504088896340736
#define PI_DIV_FOUR 0.78539816339744830961

// Keep in sync with GPUCameraData in Camera.hpp
cbuffer CameraData : register(b0, space0)
{
    float4x4 viewMatrix;
    float4x4 inverseViewMatrix;
    float4x4 projectionMatrix;
    float4x4 inverseProjectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewProjectionMatrix;
    float4 cameraPosition;
    float4 cameraCullingPosition;
    float4 cameraResolution;
    uint orthographicCamera;
    float cameraNearPlane;
    float cameraFarPlane;
    float cameraFieldOfView;
    Frustum cameraFrustum;
    Frustum cameraCullingFrustum;
    uint cameraInstanceFrustumCullingDisabled;
    uint cameraMeshletFrustumCullingDisabled;
    uint cameraMeshletBackfaceCullingDisabled;
};

cbuffer DrawData : register(b1, space0)
{
    uint materialIndex;
    uint meshletOffset;
    uint instanceOffset;
};

struct InstanceData
{
    float4x4 objectToWorld;
    uint meshletIndex;
    uint materialIndex;
    uint meshletCount;
    OBB obb;
};

StructuredBuffer<InstanceData> instanceData : register(t2, space0);

// Keep in sync with GPUMaterial in Material.hpp
struct MaterialData
{
    float3 baseColor;
    int baseColorTextureIndex;
    
    float metalness;
    int metalnessTextureIndex;
    float diffuseRoughness;
    int diffuseRoughnessTextureIndex;
    
    int normalTextureIndex;
    int ambientOcclusionTextureIndex;
    
    int padding0;
    int padding1;
};

// Bindless textures for materials
Texture2D bindlessTextures[] : register(t0, space1);
StructuredBuffer<MaterialData> materialBuffer : register(t0, space2);
SamplerState linearClampSampler : register(s0, space3);
SamplerState linearRepeatSampler : register(s1, space3);

MaterialData LoadMaterialData(uint materialIndex)
{
    return materialBuffer.Load(materialIndex);
}
InstanceData LoadInstance(uint instanceIndex, bool culling = false)
{
    InstanceData data = instanceData.Load(instanceIndex);
    
    // Apply camera relative rendering to OBB positions
    data.obb.center -= culling ? cameraCullingPosition.xyz : cameraPosition.xyz;

    return data;
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

float3 TransformObjectToWorld(float3 positionOS, float4x4 objectToWorld)
{
    return mul(float4(positionOS, 1.0), objectToWorld).xyz;
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

// Generates a triangle in homogeneous clip space, s.t.
// v0 = (-1, -1, 1), v1 = (3, -1, 1), v2 = (-1, 3, 1).
float2 GetFullScreenTriangleTexCoord(uint vertexID)
{
    return float2((vertexID << 1) & 2, (vertexID & 2));
}

float4 GetFullScreenTriangleVertexPosition(uint vertexID, float z = 1)
{
    // note: the triangle vertex position coordinates are x2 so the returned UV coordinates are in range -1, 1 on the screen.
    float2 uv = float2(vertexID & 2, (vertexID << 1) & 2);
    return float4(uv * 2.0 - 1.0, z, 1.0);
}

float2 DirectionToLatLongCoordinate(float3 unDir)
{
    float3 dir = normalize(unDir);
    // coordinate frame is (-Z, X) meaning negative Z is primary axis and X is secondary axis.
    return float2(1.0 - 0.5 * INV_PI * atan2(dir.x, -dir.z), asin(dir.y) * INV_PI + 0.5);
}

float3 LatlongToDirectionCoordinate(float2 coord)
{
    float theta = coord.y * PI;
    float phi = (coord.x * 2.f * PI - PI * 0.5f);

    float cosTheta = cos(theta);
    float sinTheta = sqrt(1.0 - min(1.0, cosTheta * cosTheta));
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float3 direction = float3(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi);
    direction.xy *= -1.0;
    return direction;
}

uint EncodeVisibility(uint materialID, uint meshletID, uint triangleID)
{
    return (materialID & 0xFF)
        | ((meshletID & 0xFFFF) << 8)
        | ((triangleID & 0xFF) << 24);
}

void DecodeVisibility(uint visibility, out uint materialID, out uint meshletID, out uint triangleID)
{
    materialID = visibility & 0xFF;
    meshletID = (visibility >> 8) & 0xFFFF;
    triangleID = visibility >> 24;
}

float3 BarycentricInterpolation(float3 v0, float3 v1, float3 v2, float2 bary)
{
    return v0 * (1 - bary.x - bary.y) + v1 * bary.x + v2 * bary.y;
}

float2 BarycentricInterpolation(float2 v0, float2 v1, float2 v2, float2 bary)
{
    return v0 * (1 - bary.x - bary.y) + v1 * bary.x + v2 * bary.y;
}
