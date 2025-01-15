#include "Mesh.hpp"

std::vector<BindingDesc> Mesh::meshletBufferBindingDescs;
std::vector<BindKey> Mesh::meshletBufferBindKeys;

template<typename T>
void Mesh::AllocateVertexBufer(std::shared_ptr<Device> device, const std::vector<T>& data, ViewType viewType, gli::format format, std::shared_ptr<Resource>& resource, std::shared_ptr<View>& view)
{
	resource = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(T) * data.size());
    resource->CommitMemory(MemoryType::kUpload);
    resource->UpdateUploadBuffer(0, data.data(), sizeof(data.front()) * data.size());
    resource->SetName(name);

    ViewDesc d = {};
    d.view_type = viewType;
    d.dimension = ViewDimension::kBuffer;
    d.buffer_format = format;
    d.structure_stride = sizeof(T);
    d.buffer_size = sizeof(T) * data.size();
    view = device->CreateView(resource, d);
}

void Mesh::BuildAndUploadMeshletData(std::shared_ptr<Device> device)
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

    AllocateVertexBufer(device, vertices, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, vertexBuffer, vertexBufferView);
    AllocateVertexBufer(device, meshlets, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, meshletsBuffer, meshletsBufferView);
    AllocateVertexBufer(device, meshletVertices, ViewType::kStructuredBuffer, gli::FORMAT_UNDEFINED, meshletIndicesBuffer, meshletIndicesBufferView);
    //AllocateVertexBufer(device, packedMeshletTriangles, ViewType::kBuffer, gli::FORMAT_R32_UINT_PACK32, meshletTrianglesBuffer, meshletTrianglesBufferView);
    AllocateVertexBufer(device, unpackedMeshletTriangles, ViewType::kBuffer, gli::FORMAT_R32_UINT_PACK32, meshletTrianglesBuffer, meshletTrianglesBufferView);

    auto vertexBufferBindKeyMesh = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 0, 4 };
    auto vertexBufferBindKeyVertex = BindKey{ ShaderType::kVertex, ViewType::kStructuredBuffer, 0, 4 };
    auto meshletsBufferBindKey = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 1, 4 };
    auto meshletIndicesBindKey = BindKey{ ShaderType::kMesh, ViewType::kBuffer, 2, 4 };
    auto meshletTrianglesBindKey = BindKey{ ShaderType::kMesh, ViewType::kBuffer, 3, 4 };

    meshletBufferBindingDescs = {
        BindingDesc{ vertexBufferBindKeyMesh, vertexBufferView },
        BindingDesc{ vertexBufferBindKeyVertex, vertexBufferView },
        BindingDesc{ meshletsBufferBindKey, meshletsBufferView },
        BindingDesc{ meshletIndicesBindKey, meshletIndicesBufferView },
        BindingDesc{ meshletTrianglesBindKey, meshletTrianglesBufferView }
    };

    meshletBufferBindKeys = {
        vertexBufferBindKeyMesh,
        vertexBufferBindKeyVertex,
        meshletsBufferBindKey,
        meshletIndicesBindKey,
        meshletTrianglesBindKey
    };

    // TODO: make a pipeline per mesh before moving everything to bindless or big f array
}

void Mesh::UploadMeshData(std::shared_ptr<Device> device)
{
	// Just before the upload, we build the meshlet data
    BuildAndUploadMeshletData(device);

	// Allocate memory for the index buffer
	indexBuffer = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * indices.size());
	indexBuffer->CommitMemory(MemoryType::kUpload);
	indexBuffer->UpdateUploadBuffer(0, indices.data(), sizeof(indices.front()) * indices.size());
    indexBuffer->SetName(name + " Index Buffer");
}

void Mesh::BindBuffers(std::shared_ptr<CommandList> commandList) const
{
    // Binding goes through the pipeline now
	commandList->IASetVertexBuffer(0, vertexBuffer);
	commandList->IASetIndexBuffer(indexBuffer, gli::FORMAT_R32_UINT_PACK32);
}