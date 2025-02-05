#include "Mesh.hpp"
#include "RenderUtils.hpp"
#include "MeshPool.hpp"

void Mesh::PrepareMeshletData(std::shared_ptr<Device> device)
{
    size_t vertexCount = positions.size();
    vertices.resize(vertexCount);
    for (size_t i = 0; i < vertexCount; i++)
    {
        vertices[i].position = positions[i];
        vertices[i].normal = normals[i];
        vertices[i].texcoord = texcoords[i];
        vertices[i].tangent = tangents[i];
    }

    // Process meshes to meshlets for the mesh shaders
    const size_t maxVertices = 64;
    const size_t maxTriangles = 124;
    const float coneWeight = 0.0f; // not used for now

    size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);
    meshlets.resize(max_meshlets);
    meshletIndices.resize(max_meshlets * maxVertices);
    meshletTriangles.resize(max_meshlets * maxTriangles * 3);

    meshletCount = meshopt_buildMeshlets(meshlets.data(), meshletIndices.data(), meshletTriangles.data(), indices.data(),
        indices.size(), (float*)positions.data(), positions.size(), sizeof(glm::vec3), maxVertices, maxTriangles, coneWeight);

    // For some reason this is making holes in the geometry so we leave it disabled for now
    // meshopt_optimizeMeshlet(meshletIndices.data(), meshletTriangles.data(), maxTriangles, maxVertices);

    // Reize the meshlet data to the actual count
    const meshopt_Meshlet& last = meshlets[meshletCount - 1];
    meshlets.resize(meshletCount);
    meshletIndices.resize(last.vertex_offset + last.vertex_count);
    meshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));

    meshletOffset = MeshPool::PushNewMesh(this);
}
