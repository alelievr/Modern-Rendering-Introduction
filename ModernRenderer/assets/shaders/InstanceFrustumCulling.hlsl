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

[numthreads(1, 1, 1)]
void clear(uint3 threadID : SV_DispatchThreadID)
{
    _VisibleMeshletsCount[0] = 0;
}

[numthreads(64, 1, 1)]
void main(uint threadID : SV_DispatchThreadID)
{
    uint instanceCout, stride;
    instanceData.GetDimensions(instanceCout, stride);
    
    if (threadID < instanceCout)
    {
        InstanceData instance = LoadInstance(threadID, true);
        
        // Frustum culling against the object OBB
        if (FrustumOBBIntersection(instance.obb, cameraCullingFrustum) || cameraInstanceFrustumCullingDisabled)
        {
            // TODO: wave interlock with surviving instances in the wavefront
            //uint argumentIndex = _IndirectCommandCounts[0];
            
            uint outStartIndex;
            InterlockedAdd(_VisibleMeshletsCount[0], instance.meshletCount, outStartIndex);
            
            
            for (uint i = 0; i < instance.meshletCount; i++)
            {
                VisibleMeshlet v;
                
                v.instanceIndex = threadID;
                v.meshletIndex = instance.meshletIndex + i;
                
                visibleMeshlets0[outStartIndex + i] = v;
            }
            
            //InterlockedCompareExchange(_IndirectCommandCounts[0], 0, 0);
        }
    }
}

[numthreads(1, 1, 1)]
void updateIndirectArguments()
{
    // TODO: support multiple commands
    uint groupCount = ceil(_VisibleMeshletsCount[0] / 64.0);

    IndirectExecuteMesh args;

    args.instanceID = 0;
    args.threadGroupX = groupCount;
    args.threadGroupY = 1;
    args.threadGroupZ = 1;

    _IndirectMeshArgs[0] = args;
    _IndirectCommandCounts[0] = 1;
}