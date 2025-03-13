#include "Common.hlsl"
#include "MeshUtils.hlsl"

struct VisibilityMeshToFragment
{
    float4 positionCS : SV_Position;
};

struct VisibilityPrimitiveAttribute
{
    uint packedVisibilityData : SV_PrimitiveID;
    bool primitiveCulled : SV_CullPrimitive;
};

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadID : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    out indices uint3 triangles[MAX_OUTPUT_PRIMITIVES],
    out vertices VisibilityMeshToFragment vertices[MAX_OUTPUT_VERTICES],
    out primitives VisibilityPrimitiveAttribute sharedPrimitives[MAX_OUTPUT_PRIMITIVES]
)
{
    VisibleMeshlet visibleMeshlet = visibleMeshlets1[groupID]; // TODO: multiple dispatch support
    uint instanceID = visibleMeshlet.instanceIndex;
    uint meshletIndex = visibleMeshlet.meshletIndex;
    
    InstanceData instance = instanceData[instanceID];
    Meshlet meshlet = meshlets[meshletIndex];

    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);

    const uint primitiveId = threadID;
    if (primitiveId < meshlet.triangleCount)
    {
        triangles[primitiveId] = LoadPrimitive(meshlet.triangleOffset, primitiveId);
        VisibilityPrimitiveAttribute attribute;
        attribute.packedVisibilityData = EncodeVisibility(groupID, threadID);
        attribute.primitiveCulled = false;
        sharedPrimitives[primitiveId] = attribute;
    }
    
    if (threadID < meshlet.vertexCount)
    {
        uint vertexIndex = meshletIndices[meshlet.vertexOffset + threadID];
        TransformedVertex vertex = LoadVertexAttributes(groupID, vertexIndex, instanceID);
        VisibilityMeshToFragment vout;
        
        vout.positionCS = vertex.positionCS;

        vertices[threadID] = vout;
    }
}

uint fragment(VisibilityMeshToFragment input, VisibilityPrimitiveAttribute prim) : SV_TARGET0
{
    return prim.packedVisibilityData;
}
