#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Instance/Instance.h"

class Mesh
{
public:
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> tangents; 

    std::shared_ptr<Resource> indexBuffer;
    std::shared_ptr<Resource> vertexPositionBuffer;
    std::shared_ptr<Resource> vertexNormalBuffer;
    std::shared_ptr<Resource> vertexTexcoordBuffer;
    std::shared_ptr<Resource> vertexTangentBuffer;

	Mesh() = default;
	~Mesh() = default;

    void UploadMeshData(std::shared_ptr<Device> device);

    void BindBuffers(std::shared_ptr<CommandList> commandList) const;

    static std::vector<InputLayoutDesc> GetInputAssemblerLayout()
    {
        return {
            { 0, "POSITION", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(glm::vec3) },
            { 1, "NORMAL", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(glm::vec3) },
            { 2, "TEXCOORD", gli::FORMAT_RG32_SFLOAT_PACK32, sizeof(glm::vec2) },
            { 3, "TANGENT", gli::FORMAT_RGB32_SFLOAT_PACK32, sizeof(glm::vec3) }
        };
    }
};