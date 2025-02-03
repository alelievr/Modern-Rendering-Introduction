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
    const size_t max_vertices = 64;
    const size_t max_triangles = 124;
    const float cone_weight = 0.0f; // not used for now

    size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles);
    meshlets.resize(max_meshlets);
    meshletVertices.resize(max_meshlets * max_vertices);
    meshletTriangles.resize(max_meshlets * max_triangles * 3);

    meshletCount = meshopt_buildMeshlets(meshlets.data(), meshletVertices.data(), meshletTriangles.data(), indices.data(),
        indices.size(), &vertices[0].position.x, vertices.size(), sizeof(Mesh::Vertex), max_vertices, max_triangles, cone_weight);

    // TODO:
    //meshopt_optimizeMeshlet(meshlet_vertices.data(), meshlet_triangles.data(), max_triangles, max_vertices);

    // Reize the meshlet data to the actual count
    const meshopt_Meshlet& last = meshlets[meshletCount - 1];
    meshlets.resize(meshletCount);
    meshletVertices.resize(last.vertex_offset + last.vertex_count);
    meshletTriangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));

    // TODO: pack 3 meshelet indices (triangle) into a single uint 
    std::vector<uint32_t> unpackedMeshletTriangles(meshletTriangles.size());
    for (int i = 0; i < meshletTriangles.size(); i++)
        unpackedMeshletTriangles[i] = meshletTriangles[i];

    // Pack meshlet triangle indices into an uint for easy read in the shader
    //std::vector<uint32_t> packedMeshletTriangles(meshletTriangles.size() / 3);

    //for (int i = 0; i < meshletCount; i++)
    //{
    //    for (int j = 0; j < max_triangles; j++)
    //    {
    //        if (j >= meshlets[i].triangle_count)
    //            break;

    //        unsigned triangleIndex = meshlets[i].triangle_offset + j;
    //        unsigned index = i * max_triangles * 3 + j * 3;
    //        uint32_t packed = 0;
    //        for (int k = 0; k < 3; k++)
    //            packed |= meshletTriangles[index + k] << (8 * k);
    //        packedMeshletTriangles[triangleIndex] = packed;
    //    }
    //}

    poolIndex = MeshPool::PushNewMesh(this);
}
