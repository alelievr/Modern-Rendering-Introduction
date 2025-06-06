#pragma once

#include "GeometryUtils.hlsl"
#include "MathUtils.hlsl"

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
    uint pathTracingFrameIndex;
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

struct RTInstanceData
{
    uint indexBufferOffset;
    uint materialIndex;
};

StructuredBuffer<InstanceData> instanceData : register(t2, space0);
StructuredBuffer<RTInstanceData> rtInstanceData : register(t3, space0);

// Keep in sync with GPUMaterial in Material.hpp
struct MaterialData
{
    float3 baseColor;
    int baseColorTextureIndex;
    
    float metalness;
    int metalnessTextureIndex;
    float diffuseRoughness;
    int diffuseRoughnessTextureIndex;
    
    float3 specularColor;
    int specularColorTextureIndex;
    
    float specularRoughness;
    int normalTextureIndex;
    int ambientOcclusionTextureIndex;
    
    int padding0;
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

float3 TransformObjectToWorldNormal(float3 normalOS, float4x4 objectToWorld)
{
    float3 normalWS = mul(normalOS, (float3x3)objectToWorld);

    return normalize(normalWS);
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
// v0 = (0, 0), v1 = (2, 0), v2 = (0, 2).
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

uint EncodeVisibility(uint visibleMeshetID, uint triangleID)
{
    return (visibleMeshetID & 0x1FFFFFF)
        | ((triangleID & 0x7F) << 25);
}

void DecodeVisibility(uint visibility, out uint visibleMeshetID, out uint triangleID)
{
    visibleMeshetID = visibility & 0x1FFFFFF;
    triangleID = visibility >> 25;
}
