#pragma once

#define PI              3.14159265358979323846
#define INV_PI          0.31830988618379067154
#define HALF_PI         1.57079632679489661923
#define LOG2_E          1.44269504088896340736
#define PI_DIV_FOUR     0.78539816339744830961
#define MAX_UINT        0xFFFFFFFFu
#define FLOAT_EPSYLON   1e-5

float Sq(float x)
{
    return x * x;
}

float2 Sq(float2 x)
{
    return x * x;
}

float3 Sq(float3 x)
{
    return x * x;
}

float4 Sq(float4 x)
{
    return x * x;
}

float SafeDiv(float numer, float denom)
{
    return (numer != denom) ? numer / denom : 1;
}
