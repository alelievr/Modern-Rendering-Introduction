#pragma once

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

float3 RandomNormalizedVector(uint3 seed)
{
    float3 r = FloatHash33(seed);
    
    
    return r;
}