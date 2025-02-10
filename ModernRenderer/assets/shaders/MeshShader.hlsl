#include "Common.hlsl"
#include "MeshUtils.hlsl"

struct VertexData
{
    float3 positionOS;
    float3 normal;
    float2 uv;
    float3 tangent;
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

// TODO: single vertex buffer for all meshes
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

// must match meshlet generation limits
#define MAX_OUTPUT_VERTICES 128
#define MAX_OUTPUT_PRIMITIVES 256

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint threadId : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    uint groupIndex : SV_GroupIndex,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices MeshToFragment vertices[MAX_OUTPUT_VERTICES])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    Meshlet meshlet = meshlets[groupID + meshletOffset];
    uint instanceID = groupIndex / meshlet.vertexCount;

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    for (uint i = 0; i < 2; ++i)
    {
        const uint primitiveId = threadId + i * 128;
        if (primitiveId < meshlet.triangleCount)
        {
            triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
        }
    }
    
    if (threadId < meshlet.vertexCount)
    {
        uint vertexIndex = meshletIndices[meshlet.vertexoffset + threadId];

        vertices[threadId] = LoadVertexAttributes(groupID, vertexIndex, instanceID);
    }
}