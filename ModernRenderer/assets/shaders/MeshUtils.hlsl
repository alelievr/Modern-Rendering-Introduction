#pragma once

#include "Common.hlsl"

// must match meshlet generation limits
#define MAX_OUTPUT_VERTICES 128
#define MAX_OUTPUT_PRIMITIVES 256

struct VertexData
{
    float3 positionOS;
    float3 normal;
    float2 uv;
    float3 tangent;
};

struct MeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    nointerpolation float meshletIndex : TEXCOORD2;
};

struct VisibleMeshlet
{
    uint meshletID;
    uint instanceID;
};

// From meshoptimizer
struct Meshlet
{
	/* offsets within meshlet_vertices and meshlet_triangles arrays with meshlet data */
    unsigned int vertexoffset;
    unsigned int triangleOffset;

	/* number of vertices and triangles used in the meshlet; data is stored in consecutive range defined by offset and count */
    unsigned int vertexCount;
    unsigned int triangleCount;
};

// From meshopt_Bounds
struct Bounds
{
	/* bounding sphere, useful for frustum and occlusion culling */
    float3 center;
    float radius;

	/* normal cone, useful for backface culling */
    float3 coneApex;
    float3 coneAxis;
    float coneCutoff; /* = cos(angle/2) */

    uint coneAxisAndCutoff;
};

StructuredBuffer<VertexData> vertexBuffer : register(t0, space4);
StructuredBuffer<Meshlet> meshlets : register(t1, space4);
Buffer<uint> meshletIndices : register(t2, space4);
Buffer<uint> meshletTriangles : register(t3, space4);
StructuredBuffer<Bounds> meshletBounds : register(t4, space4);

Bounds LoadMeshletBounds(uint meshletIndex, bool culling = false)
{
    Bounds bounds = meshletBounds[meshletIndex];
    
    bounds.center -= culling ? cameraCullingPosition.xyz : cameraPosition.xyz;
    
    return bounds;
}

MeshToFragment LoadVertexAttributes(uint meshletIndex, uint vertexIndex, uint instanceID)
{
    MeshToFragment vout;
    
    // Fetch mesh data from buffers
    VertexData vertex = vertexBuffer.Load(vertexIndex);
    InstanceData instance = LoadInstance(instanceID);
    
    // Apply camera relative rendering
    vertex.positionOS = GetCameraRelativePosition(vertex.positionOS);
    
    float3 positionWS = TransformObjectToWorld(vertex.positionOS, instance.objectToWorld);
    
    vout.positionCS = TransformCameraRelativeWorldToHClip(positionWS);
    vout.uv = vertex.uv;
    vout.meshletIndex = meshletIndex;
    vout.normal = vertex.normal; // TODO: transform normal

    return vout;
}

uint3 LoadPrimitive(uint offset, uint threadId)
{
    // TODO: compation instead of 3 loads
    return uint3(
        meshletTriangles[offset + threadId * 3 + 0],
        meshletTriangles[offset + threadId * 3 + 1],
        meshletTriangles[offset + threadId * 3 + 2]
    );
}
