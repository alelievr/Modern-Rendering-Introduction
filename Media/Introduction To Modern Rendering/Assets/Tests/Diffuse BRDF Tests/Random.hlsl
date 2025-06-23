#pragma once

// Building a Fast, SIMD/GPU-friendly Random Number Generator For Fun And Profit
// https://vectrx.substack.com/p/lcg-xs-fast-gpu-rng

uint4 randomGlobalState;

void InitRandomSeed(uint seed)
{
    // several iterations can be run to further decorrelate threads
    for (uint i = 0; i < 3; i += 1)
    {
        seed = seed * 2654435761u + 1692572869u;
        seed = seed ^ (seed >> 18);
    }

    randomGlobalState.x = seed;

    for (i = 0; i < 3; i += 1)
    {
        seed = seed * 2654435761u + 1692572869u;
        seed = seed ^ (seed >> 18);
    }

    randomGlobalState.y = seed;

    for (i = 0; i < 3; i += 1)
    {
        seed = seed * 2654435761u + 1692572869u;
        seed = seed ^ (seed >> 18);
    }

    randomGlobalState.z = seed;

    for (i = 0; i < 3; i += 1)
    {
        seed = seed * 2654435761u + 1692572869u;
        seed = seed ^ (seed >> 18);
    }

    randomGlobalState.w = seed;
}

void InitRandomSeed(uint4 seeds)
{
    for (uint i = 0; i < 3; i += 1)
    {
        seeds = seeds * 2654435761u + 1692572869u;
        seeds = seeds ^ (seeds >> 18);
    }

    randomGlobalState = seeds;
}

uint lcg_xs_24(inout uint state)
{
    uint result = state * 747796405u + 2891336453u;
    uint hashed_result = result ^ (result >> 14);
    state = hashed_result;
    return hashed_result >> 8;
}

float random(inout uint state)
{
    uint result = lcg_xs_24(state);
    const float inv_max_int = 1.0 / 16777216.0;
    return float(result) * inv_max_int;
}

float NextRandomFloat()
{
    return random(randomGlobalState.x);
}

float2 NextRandomFloat2()
{
    return float2(random(randomGlobalState.x), random(randomGlobalState.y));
}

float3 NextRandomFloat3()
{
    return float3(random(randomGlobalState.x), random(randomGlobalState.y), random(randomGlobalState.z));
}

float4 NextRandomFloat4()
{
    return float4(random(randomGlobalState.x), random(randomGlobalState.y), random(randomGlobalState.z), random(randomGlobalState.w));
}