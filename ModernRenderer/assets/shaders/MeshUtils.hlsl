#pragma once

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

StructuredBuffer<VertexData> vertexBuffer : register(t0, space4);
StructuredBuffer<Meshlet> meshlets : register(t1, space4);
Buffer<uint> meshletIndices : register(t2, space4);
Buffer<uint> meshletTriangles : register(t3, space4);

MeshToFragment LoadVertexAttributes(uint meshletIndex, uint vertexIndex, uint instanceID)
{
    MeshToFragment vout;
    
    // Fetch mesh data from buffers
    VertexData vertex = vertexBuffer.Load(vertexIndex);
    InstanceData instance = instanceData.Load(instanceOffset + instanceID);
    
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
