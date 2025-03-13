#include "MeshPool.hpp"
#include "RenderUtils.hpp"

std::unordered_map<std::shared_ptr<Mesh>, unsigned> MeshPool::meshes;

std::vector<meshopt_Meshlet> MeshPool::meshlets;
std::vector<uint32_t> MeshPool::meshletIndices;
std::vector<uint8_t> MeshPool::meshletTriangles;
std::vector<Mesh::Vertex> MeshPool::vertices;
std::vector<uint32_t> MeshPool::indices;
std::vector<meshopt_Bounds> MeshPool::bounds;

std::shared_ptr<Resource> MeshPool::vertexPool = nullptr;
std::shared_ptr<Resource> MeshPool::indicesPool = nullptr;
std::shared_ptr<Resource> MeshPool::meshletIndicesPool = nullptr;
std::shared_ptr<Resource> MeshPool::meshletTrianglesPool = nullptr;
std::shared_ptr<Resource> MeshPool::meshletsPool = nullptr;
std::shared_ptr<Resource> MeshPool::meshletBoundsPool = nullptr;

std::shared_ptr<View> MeshPool::vertexPoolView = nullptr;
std::shared_ptr<View> MeshPool::indicesPoolView = nullptr;
std::shared_ptr<View> MeshPool::meshletIndicesPoolView = nullptr;
std::shared_ptr<View> MeshPool::meshletTrianglesPoolView = nullptr;
std::shared_ptr<View> MeshPool::meshletsPoolView = nullptr;
std::shared_ptr<View> MeshPool::meshletBoundsPoolView;

std::vector<BindingDesc> MeshPool::bindingDescs;
std::vector<BindKey> MeshPool::bindKeys;

unsigned MeshPool::PushNewMesh(std::shared_ptr<Mesh> mesh)
{
    unsigned meshletOffset = meshlets.size();

    auto f = meshes.find(mesh);
    if (f == meshes.end())
        meshes.insert({ mesh, meshletOffset });
	else
		return f->second;

    // Append vertices to the pool, storing the index offset to update meshlet data
    int vertexOffset = vertices.size();
	vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
    int triangleOffset = meshletTriangles.size();
	meshletTriangles.insert(meshletTriangles.end(), mesh->meshletTriangles.begin(), mesh->meshletTriangles.end());
    int indexOffset = meshletIndices.size();
    bounds.insert(bounds.end(), mesh->meshletBounds.begin(), mesh->meshletBounds.end());
    int indicesOffset = indices.size();
    indices.reserve(indices.size() + mesh->indices.size());
    for (int i = 0; i < mesh->indices.size(); i++)
        indices.push_back(mesh->indices[i] + vertexOffset);

    for (const auto& index : mesh->meshletIndices)
		meshletIndices.push_back(index + vertexOffset);

    for (const auto& meshet : mesh->meshlets)
	{
		meshopt_Meshlet newMeshlet = meshet;
		newMeshlet.vertex_offset += indexOffset;
        newMeshlet.triangle_offset += triangleOffset;
		meshlets.push_back(newMeshlet);
	}

    mesh->meshletOffset = meshletOffset;
    mesh->raytracedPrimitiveIndex = indicesOffset;

    return meshletOffset;
}

void MeshPool::AllocateMeshPoolBuffers(std::shared_ptr<Device> device)
{
	RenderUtils::AllocateVertexBufer(device, vertices, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, "Vertex Pool", vertexPool, vertexPoolView);
	RenderUtils::AllocateVertexBufer(device, meshlets, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, "Meshlet Pool", meshletsPool, meshletsPoolView);
	RenderUtils::AllocateVertexBufer(device, meshletIndices, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, "Meshlet Vertices", meshletIndicesPool, meshletIndicesPoolView);
    RenderUtils::AllocateVertexBufer(device, bounds, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, "Meshlet Bounds", meshletBoundsPool, meshletBoundsPoolView);
    RenderUtils::AllocateVertexBufer(device, indices, ViewType::kBuffer, gli::FORMAT_R32_UINT_PACK32, "Vertex Indices", indicesPool, indicesPoolView);

    // TODO: pack 3 meshelet indices (triangle) into a single uint 
    std::vector<uint32_t> unpackedMeshletTriangles(meshletTriangles.size());
    for (int i = 0; i < meshletTriangles.size(); i++)
        unpackedMeshletTriangles[i] = meshletTriangles[i];

    //RenderUtils::AllocateVertexBufer(device, packedMeshletTriangles, ViewType::kBuffer, gli::FORMAT_R32_UINT_PACK32, meshletTrianglesBuffer, meshletTrianglesBufferView);
	RenderUtils::AllocateVertexBufer(device, unpackedMeshletTriangles, ViewType::kBuffer, gli::FORMAT_R32_UINT_PACK32, "Meshlet Triangles", meshletTrianglesPool, meshletTrianglesPoolView);

    auto vertexPoolBindKeyMesh = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 0, 4 };
    auto meshletsPoolBindKey = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 1, 4 };
    auto meshletIndicesBindKey = BindKey{ ShaderType::kMesh, ViewType::kBuffer, 2, 4 };
    auto meshletTrianglesBindKey = BindKey{ ShaderType::kMesh, ViewType::kBuffer, 3, 4 };
    auto meshletBoundsBindKey = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 4, 4 };
    auto indicesBindKey = BindKey{ ShaderType::kMesh, ViewType::kBuffer, 5, 4 };

    bindingDescs = {
        BindingDesc{ vertexPoolBindKeyMesh, vertexPoolView },
        BindingDesc{ meshletsPoolBindKey, meshletsPoolView },
        BindingDesc{ meshletIndicesBindKey, meshletIndicesPoolView },
        BindingDesc{ meshletTrianglesBindKey, meshletTrianglesPoolView },
        BindingDesc{ meshletBoundsBindKey, meshletBoundsPoolView },
        BindingDesc{ indicesBindKey, indicesPoolView },
    };

    bindKeys = {
        vertexPoolBindKeyMesh,
        meshletsPoolBindKey,
        meshletIndicesBindKey,
        meshletTrianglesBindKey,
        meshletBoundsBindKey,
        indicesBindKey,
    };
}
