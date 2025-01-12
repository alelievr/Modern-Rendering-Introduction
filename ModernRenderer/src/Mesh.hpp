#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include "meshoptimizer.h"
#include "Instance/Instance.h"

class Mesh
{
private:

    void BuildAndUploadMeshletData(std::shared_ptr<Device> device);

public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec3 tangent;
    };

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> tangents;

    // Meshlet data
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletVertices;
    std::vector<uint8_t> meshletTriangles;
    size_t meshletCount;

    std::shared_ptr<Resource> indexBuffer;
    std::shared_ptr<Resource> vertexBuffer;
    std::shared_ptr<Resource> meshletIndicesBuffer;
    std::shared_ptr<Resource> meshletTrianglesBuffer;
    std::shared_ptr<Resource> meshletsBuffer;

    std::shared_ptr<View> vertexBufferView;
    std::shared_ptr<View> meshletIndicesBufferView;
    std::shared_ptr<View> meshletTrianglesBufferView;
    std::shared_ptr<View> meshletsBufferView;

    static std::vector<BindingDesc> meshletBufferBindingDescs;
    static std::vector<BindKey> meshletBufferBindKeys;

	Mesh() = default;
	~Mesh() = default;

    void UploadMeshData(std::shared_ptr<Device> device);

    void BindBuffers(std::shared_ptr<CommandList> commandList) const;

    static std::vector<InputLayoutDesc> GetInputAssemblerLayout()
    {
        return {
            { 0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(Vertex) },
            { 0, "NORMAL", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(Vertex) },
            { 0, "TEXCOORD", gli::FORMAT_RG32_SFLOAT_PACK32, sizeof(Vertex) },
            { 0, "TANGENT", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(Vertex) }
        };
    }
};