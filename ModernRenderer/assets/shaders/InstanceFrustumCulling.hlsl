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

RWBuffer<uint> _VisibleInstanceCount : register(u1, space0);
RWStructuredBuffer<IndirectExecuteMesh> _IndirectMeshArgs : register(u2, space0);

[numthreads(1, 1, 1)]
void clear(uint3 threadID : SV_DispatchThreadID)
{
    _VisibleInstanceCount[0] = 0;
}

[numthreads(64, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint instanceCout, stride;
    instanceData.GetDimensions(instanceCout, stride);
    
    if (threadID.x < instanceCout)
    {
        InstanceData instance = instanceData.Load(threadID.x);
        
        // Frustum culling against the object OBB
        if (FrustumOBBIntersection(instance.obb, cameraFrustum) || cameraFrustumCullingDisabled)
        {
            // TODO Backface culling
            
            // TODO: wave interlock with surviving instances in the wavefront
            uint outIndex;
            InterlockedAdd(_VisibleInstanceCount[0], 1, outIndex);
        
            IndirectExecuteMesh visibleInstance;
        
            visibleInstance.instanceID = threadID.x;
            visibleInstance.threadGroupX = instance.meshletCount;
            visibleInstance.threadGroupY = 1;
            visibleInstance.threadGroupZ = 1;
        
            _IndirectMeshArgs[outIndex] = visibleInstance;
        }
    }
}