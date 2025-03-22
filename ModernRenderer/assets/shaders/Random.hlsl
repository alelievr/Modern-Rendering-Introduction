#pragma once

#include "Common.hlsl"

// Construct a float with half-open range [0, 1) using low 23 bits.
// All zeros yields 0, all ones yields the next smallest representable value below 1.
float ConstructFloat(int m)
{
    const int ieeeMantissa = 0x007FFFFF; // Binary FP32 mantissa bitmask
    const int ieeeOne = 0x3F800000; // 1.0 in FP32 IEEE

    m &= ieeeMantissa; // Keep only mantissa bits (fractional part)
    m |= ieeeOne; // Add fractional part to 1.0

    float f = asfloat(m); // Range [1, 2)
    return f - 1; // Range [0, 1)
}

float ConstructFloat(uint m)
{
    return ConstructFloat(asint(m));
}

// https://github.com/skeeto/hash-prospector
uint IntegerHash11(uint x)
{
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0xd35a2d97;
    x ^= x >> 15;
    return x;
}

uint IntegerHash21(uint2 x)
{
    return IntegerHash11(x.x ^ IntegerHash11(x.y));
}

uint2 IntegerHash22(uint2 x)
{
    return uint2(IntegerHash11(x.x), IntegerHash11(x.y));
}

uint3 IntegerHash33(uint3 x)
{
    return uint3(IntegerHash11(x.x), IntegerHash11(x.y), IntegerHash11(x.z));
}

float FloatHash21(float2 c)
{
    return ConstructFloat(IntegerHash21(asuint(c)));
}

float2 FloatHash22(float2 c)
{
    uint2 hash = IntegerHash22(asuint(c));
    
    return float2(
        ConstructFloat(hash.x),
        ConstructFloat(hash.y)
    );
}

float2 FloatHash32(float3 c)
{
    uint3 u = asuint(c);
    uint2 hash = uint2(IntegerHash21(u.xy), IntegerHash21(u.yz ^ IntegerHash11(u.x)));
    return float2(ConstructFloat(hash.x), ConstructFloat(hash.y));
}

 // Random between 0 and 1
float2 FloatHash22(uint2 c)
{
    uint2 hash = IntegerHash22(c);
    
    return float2(
        ConstructFloat(hash.x),
        ConstructFloat(hash.y)
    );
}

float3 FloatHash33(uint3 c)
{
    uint3 hash = IntegerHash33(c);
    
    return float3(
        ConstructFloat(hash.x),
        ConstructFloat(hash.y),
        ConstructFloat(hash.z)
    );
}

float3 RandomNormalizedVector(float3 x)
{
    float2 h = FloatHash32(x + float3(pathTracingFrameIndex * 0.001, pathTracingFrameIndex * 0.0017, pathTracingFrameIndex * 0.0023));
    float2 xi = FloatHash22(h);
    float z = 1.0 - 2.0 * xi.x;
    float phi = 2.0 * PI * xi.y;
    float r = sqrt(max(0.0, 1.0 - z * z));
    return float3(r * cos(phi), r * sin(phi), z);
}

float3 RandomHemisphereVector(float3 x, float3 n)
{
    float3 r = RandomNormalizedVector(x);
    
    if (dot(r, n) < 0)
        r = -r;
    
    return r;
}
