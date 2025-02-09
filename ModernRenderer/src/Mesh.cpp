#include "Mesh.hpp"
#include "RenderUtils.hpp"
#include "MeshPool.hpp"
#include <Utilities/Common.h>
#include "QueryHeap/DXRayTracingQueryHeap.h"

void Mesh::PrepareMeshletData(std::shared_ptr<Device> device)
{
    size_t vertexCount = positions.size();
    std::vector<Vertex> tmpVertices;
    tmpVertices.resize(vertexCount);
    for (size_t i = 0; i < vertexCount; i++)
    {
        tmpVertices[i].position = positions[i];
        tmpVertices[i].normal = normals[i];
        tmpVertices[i].texcoord = texcoords[i];
        tmpVertices[i].tangent = tangents[i];
    }

    // Optimize for vertex cache
    vertices.resize(vertexCount);
    meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), tmpVertices.data(), vertexCount, sizeof(Vertex));

    // Process meshes to meshlets for the mesh shaders
    const size_t maxVertices = 128;
    const size_t maxTriangles = 256;
    const float coneWeight = 0.8f; // not used for now

    size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);
    meshlets.resize(max_meshlets);
    meshletIndices.resize(max_meshlets * maxVertices);
    meshletTriangles.resize(max_meshlets * maxTriangles * 3);

    meshletCount = meshopt_buildMeshlets(meshlets.data(), meshletIndices.data(), meshletTriangles.data(), indices.data(),
        indices.size(), (float*)vertices.data(), vertices.size(), sizeof(Vertex), maxVertices, maxTriangles, coneWeight);

    // TODO: test perfs of this
    for (size_t i = 0; i < meshlets.size(); ++i)
    {
        const meshopt_Meshlet& meshlet = meshlets[i];

        meshopt_optimizeMeshlet(&meshletIndices[meshlet.vertex_offset], &meshletTriangles[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);
    }

    // Reize the meshlet data to the actual count
    const meshopt_Meshlet& last = meshlets[meshletCount - 1];
    meshlets.resize(meshletCount);
    meshletIndices.resize(last.vertex_offset + last.vertex_count);
    meshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));

    meshletOffset = MeshPool::PushNewMesh(this);
}

void Mesh::PrepareBLASData(std::shared_ptr<Device> device)
{
    // upload first mesh data
    rtVertexPositions = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, positions.size() * sizeof(positions[0]));
    rtVertexPositions->CommitMemory(MemoryType::kDefault);
    rtVertexPositions->SetName("RT Vertex Positions: " + name);
    RenderUtils::UploadBufferData(device, rtVertexPositions, positions.data(), positions.size() * sizeof(positions[0]));
    // Upload first mesh index data
    rtIndexBuffer = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, indices.size() * sizeof(indices[0]));
    rtIndexBuffer->CommitMemory(MemoryType::kDefault);
    rtIndexBuffer->SetName("RT Index Buffer: " + name);
    RenderUtils::UploadBufferData(device, rtIndexBuffer, indices.data(), indices.size() * sizeof(indices[0]));

    geometryDesc = {
        { rtVertexPositions, gli::format::FORMAT_RGB32_SFLOAT_PACK32, (unsigned)positions.size() },
        { rtIndexBuffer, gli::format::FORMAT_R32_UINT_PACK32, (unsigned)indices.size() },
        RaytracingGeometryFlags::kOpaque,
    };

    blasPrebuildInfo = device->GetBLASPrebuildInfo({ geometryDesc }, BuildAccelerationStructureFlags::kAllowCompaction);
}

std::shared_ptr<Resource> Mesh::CreateBLAS(std::shared_ptr<Device> device, std::shared_ptr<Resource> blasBuffer, uint64_t offset, std::shared_ptr<Resource> scratch)
{
    auto cmd = device->CreateCommandList(CommandListType::kGraphics);
    cmd->SetName("BLAS Build Command List");
    uint64_t fenceValue = 0;
    std::shared_ptr<Fence> fence = device->CreateFence(fenceValue);
    auto uploadQueue = device->GetCommandQueue(CommandListType::kGraphics);

    // Allocate tmp blas for the non compacted version
    uint64_t blasSize = Align(blasPrebuildInfo.acceleration_structure_size, kAccelerationStructureAlignment);
    auto tmpBlasBuffer = device->CreateBuffer(BindFlag::kAccelerationStructure, blasSize);
    tmpBlasBuffer->CommitMemory(MemoryType::kDefault);
    tmpBlasBuffer->SetName("Bottom Level Acceleration Structures");

    blas = device->CreateAccelerationStructure(AccelerationStructureType::kBottomLevel, blasBuffer, offset);
    auto tmpBlas = device->CreateAccelerationStructure(AccelerationStructureType::kBottomLevel, tmpBlasBuffer, 0);

    auto blas_compacted_size_buffer = device->CreateBuffer(BindFlag::kCopyDest, sizeof(uint64_t));
    blas_compacted_size_buffer->CommitMemory(MemoryType::kReadback);
    blas_compacted_size_buffer->SetName("blas_compacted_size_buffer");

    auto queryHeap = device->CreateQueryHeap(QueryHeapType::kAccelerationStructureCompactedSize, 1);
    DXRayTracingQueryHeap* dxQueryHeap = (DXRayTracingQueryHeap*)queryHeap.get();
    dxQueryHeap->GetResource()->SetName(L"BLAS Compacted Size Query Heap");

    cmd->BuildBottomLevelAS({}, tmpBlas, scratch, 0, { geometryDesc },
        BuildAccelerationStructureFlags::kAllowCompaction);
    cmd->UAVResourceBarrier(tmpBlas);
    cmd->WriteAccelerationStructuresProperties({ tmpBlas }, queryHeap, 0);
    cmd->ResolveQueryData(queryHeap, 0, 1, blas_compacted_size_buffer, 0);
    
    cmd->UAVResourceBarrier(tmpBlas);
    cmd->CopyAccelerationStructure(tmpBlas, blas, CopyAccelerationStructureMode::kCompact);
    
    cmd->Close();

    uploadQueue->ExecuteCommandLists({ cmd });
    uploadQueue->Signal(fence, ++fenceValue);
    fence->Wait(fenceValue);
    uploadQueue->Wait(fence, fenceValue);

    blas_compacted_size = *reinterpret_cast<uint64_t*>(blas_compacted_size_buffer->Map());
    blas_compacted_size_buffer->Unmap();

    return blas;
}
