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

MeshToFragment GetVertexAttributes(uint meshletIndex, uint vertexIndex, uint instanceID)
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

    return vout;
}

// must match meshlet generation limits
#define MAX_OUTPUT_VERTICES 64
#define MAX_OUTPUT_PRIMITIVES 124

[NumThreads(MAX_OUTPUT_PRIMITIVES, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint threadId : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    uint groupIndex : SV_GroupIndex,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices MeshToFragment vertices[MAX_OUTPUT_VERTICES])
{
    Meshlet meshlet = meshlets[groupID + meshletOffset];
    uint instanceID = groupIndex / meshlet.vertexCount;

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    if (threadId < meshlet.triangleCount)
    {
        uint vIdx0 = meshletTriangles[meshlet.triangleOffset + threadId * 3 + 0];
        uint vIdx1 = meshletTriangles[meshlet.triangleOffset + threadId * 3 + 1];
        uint vIdx2 = meshletTriangles[meshlet.triangleOffset + threadId * 3 + 2];
        //uint vIdx0  = (packed >>  0) & 0xFF;
        //uint vIdx1  = (packed >>  8) & 0xFF;
        //uint vIdx2  = (packed >> 16) & 0xFF;
        triangles[threadId] = uint3(vIdx0, vIdx1, vIdx2);
    }

    if (threadId < meshlet.vertexCount)
    {
        uint vertexIndex = meshlet.vertexoffset + threadId;
        vertexIndex = meshletIndices[vertexIndex];

        vertices[threadId] = GetVertexAttributes(groupID, vertexIndex, instanceID);
        
        //float3 color = float3(
        //    float(gid & 1),
        //    float(gid & 3) / 4,
        //    float(gid & 7) / 8);
    }
}