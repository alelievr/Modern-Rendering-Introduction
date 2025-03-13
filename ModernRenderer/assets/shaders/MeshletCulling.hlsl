#include "Common.hlsl"
#include "MeshUtils.hlsl"
#include "GeometryUtils.hlsl"

// Custom indirect struct to dispatch mesh shader with the instance ID patched as CBuffer
struct IndirectExecuteMesh
{
    uint instanceID;
    uint threadGroupX;
    uint threadGroupY;
    uint threadGroupZ;
};

RWBuffer<uint> _IndirectCommandCounts : register(u1, space0);
RWStructuredBuffer<IndirectExecuteMesh> _IndirectMeshArgs : register(u2, space0);
RWBuffer<uint> _VisibleMeshletsCount : register(u3, space0);

#define VISIBLE_MESHLET_COUNT_KEY 15

[numthreads(1, 1, 1)]
void clear()
{
    _VisibleMeshletsCount[VISIBLE_MESHLET_COUNT_KEY] = 0;
}

[numthreads(64, 1, 1)]
void main(uint threadID : SV_DispatchThreadID)
{
    uint visibleMeshletIndex = threadID; // TODO: support over max 65k meshlets on screen
    
    if (visibleMeshletIndex < _VisibleMeshletsCount[0])
    {
        VisibleMeshlet visibleMeshlet = visibleMeshlets0[visibleMeshletIndex];
    
        InstanceData instance = instanceData[visibleMeshlet.instanceIndex];
        bool visible = true;
    
        Bounds bounds = LoadMeshletBounds(visibleMeshlet.meshletIndex, true);
    
        // TODO: transform bounds to world space
        bounds.center = TransformObjectToWorld(bounds.center, instance.objectToWorld);
        bounds.coneApex = TransformObjectToWorld(bounds.coneApex, instance.objectToWorld);
    
        // Perform cone culling to eliminate backfacing meshlets
        if (!cameraMeshletBackfaceCullingDisabled)
            if (dot(normalize(bounds.coneApex - cameraCullingPosition.xyz), bounds.coneAxis) >= bounds.coneCutoff)
                visible = false;

        // Perform frustum culling on the meshlet
        if (!cameraMeshletFrustumCullingDisabled)
            if (visible && SphereFrustumIntersection(cameraCullingFrustum, bounds.center, bounds.radius) <= 0)
                visible = false;
    
        if (visible)
        {
            // TODO: wave compaction
            uint writeIndex;
            // TODO: support over max 65k meshlets on screen
            InterlockedAdd(_VisibleMeshletsCount[VISIBLE_MESHLET_COUNT_KEY], 1, writeIndex);
            visibleMeshlets1[writeIndex] = visibleMeshlet;
        }
    }
}

[numthreads(1, 1, 1)]
void updateIndirectArguments()
{
    uint groupCount = _VisibleMeshletsCount[VISIBLE_MESHLET_COUNT_KEY]; // TODO: support multiple commands
    
    IndirectExecuteMesh args;

    args.instanceID = 0;
    args.threadGroupX = groupCount;
    args.threadGroupY = 1;
    args.threadGroupZ = 1;
    _IndirectMeshArgs[0] = args;
    _IndirectCommandCounts[0] = 1;
}