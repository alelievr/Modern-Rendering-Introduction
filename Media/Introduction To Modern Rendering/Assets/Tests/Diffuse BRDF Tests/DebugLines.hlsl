#pragma once

struct DebugLine
{
    float3 start;
    float3 end;
    float4 color;
};

struct IndirectDraw
{
    uint vertexCount;
    uint instanceCount;
    uint startVertexLocation;
    uint startInstanceLocation;
};

#ifdef READ_DEBUG
StructuredBuffer<DebugLine> _DebugLines;
#else
RWStructuredBuffer<DebugLine> _DebugLines;
#endif

RWStructuredBuffer<IndirectDraw> _IndirectLineDrawArgs;

void AddDebugLine(float3 start, float3 end, float4 color)
{
#ifndef READ_DEBUG
    DebugLine debugLine;
    debugLine.start = start;
    debugLine.end = end;
    debugLine.color = color;

    uint index = 0;
    InterlockedAdd(_IndirectLineDrawArgs[0].instanceCount, 1, index);

    _DebugLines[index] = debugLine;
#endif
}

DebugLine GetDebugLine(uint index)
{
    return _DebugLines[index];
}