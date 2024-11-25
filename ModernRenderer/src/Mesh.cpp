#include "Mesh.hpp"

template<typename T>
std::shared_ptr<Resource> AllocateVertexBufer(std::shared_ptr<Device> device, const std::vector<T>& data)
{
	auto buffer = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(T) * data.size());
	buffer->CommitMemory(MemoryType::kUpload);
	buffer->UpdateUploadBuffer(0, data.data(), sizeof(data.front()) * data.size());
	return buffer;
}

void Mesh::BuildAndUploadMeshletData(std::shared_ptr<Device> device)
{
    vertices.resize(positions.size());
    for (size_t i = 0; i < positions.size(); i++)
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
    meshlets.resize(meshletCount);
    meshletVertices.resize(meshletCount * max_vertices);
    meshletTriangles.resize(meshletCount * max_triangles * 3);

    // Pack meshlet triangle indices into an uint for easy read in the shader
    std::vector<uint32_t> packedMeshletTriangles(meshletCount * max_triangles * 3);
    for (int i = 0; i < max_meshlets * max_triangles; i++)
    {
        uint32_t packed = 0;
        for (int j = 0; j < 3; j++)
			packed |= meshletTriangles[i * 3 + j] << (8 * j);
		packedMeshletTriangles[i] = packed;
    }

    vertexBuffer = AllocateVertexBufer(device, vertices);
    meshletsBuffer = AllocateVertexBufer(device, meshlets);
    meshletIndicesBuffer = AllocateVertexBufer(device, meshletVertices);
    meshletTrianglesBuffer = AllocateVertexBufer(device, packedMeshletTriangles);
}

void Mesh::UploadMeshData(std::shared_ptr<Device> device)
{
	// Just before the upload, we build the meshlet data
    BuildAndUploadMeshletData(device);

	// Allocate memory for the index buffer
	indexBuffer = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * indices.size());
	indexBuffer->CommitMemory(MemoryType::kUpload);
	indexBuffer->UpdateUploadBuffer(0, indices.data(), sizeof(indices.front()) * indices.size());
}

void Mesh::BindBuffers(std::shared_ptr<CommandList> commandList) const
{
    // Binding goes through the pipeline now
	//commandList->IASetVertexBuffer(0, vertexBuffer);
	//commandList->IASetIndexBuffer(indexBuffer, gli::FORMAT_R32_UINT_PACK32);
}