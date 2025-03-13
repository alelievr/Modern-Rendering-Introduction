#include "Common.hlsl"
#include "MeshUtils.hlsl"

Texture2D<uint> _VisibilityTexture : register(t1, space2);

struct ForwardMeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ForwardMeshToFragment GetFullscreenTriangleVertex(uint id)
{
    ForwardMeshToFragment v;
    
    v.positionCS = GetFullScreenTriangleVertexPosition(id);
    v.uv = GetFullScreenTriangleTexCoord(id);

    return v;
}

[NumThreads(1, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    uint threadId : SV_GroupThreadID,
    uint groupID : SV_GroupID,
    uint groupIndex : SV_GroupIndex,
    out indices uint3 triangles[1],
    out vertices ForwardMeshToFragment vertices[3])
    // TODO: use primitive attributes to send meshlet ID to shaders
{
    SetMeshOutputCounts(3, 1);
    
    triangles[0] = uint3(0, 1, 2);
    vertices[0] = GetFullscreenTriangleVertex(0);
    vertices[1] = GetFullscreenTriangleVertex(1);
    vertices[2] = GetFullscreenTriangleVertex(2);
}

float4 fragment(ForwardMeshToFragment input) : SV_TARGET0
{
    uint visibilityData = _VisibilityTexture.Load(uint3(input.positionCS.xy, 0));
    
    uint visibleMeshetID, triangleID;
    DecodeVisibility(visibilityData, visibleMeshetID, triangleID);
    
    VisibleMeshlet visibleMeshlet = visibleMeshlets1[visibleMeshetID];
    
    InstanceData instance = LoadInstance(visibleMeshlet.instanceIndex);
    
    //Meshlet meshlet = meshlets[visibleMeshlet.meshletIndex];
    //uint3 primitiveIndices = LoadPrimitive(meshlet.triangleOffset, triangleID);
    //MeshToFragment attibs = LoadVertexAttributes(visibleMeshlet.meshletIndex, primitiveIndices.x, visibleMeshlet.instanceIndex);
    
    //// Cast ray, find barycentric coords, interpolate vertex data
    
    //// Shade material surface with lighting
    //MaterialData material = materialBuffer.Load(instance.materialIndex);
    
    // Apply fog
    
    return float4(GetRandomColor(visibleMeshetID), 1);
}
