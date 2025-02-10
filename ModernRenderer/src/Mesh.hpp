#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include "meshoptimizer.h"
#include "Instance/Instance.h"

class Mesh
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        glm::vec3 tangent;
    };

    std::string name;

    // Mesh data
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> tangents;

    // Meshlet data
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletIndices;
    std::vector<uint8_t> meshletTriangles;
    size_t meshletCount;

    // Raytracing data
    RaytracingASPrebuildInfo blasPrebuildInfo;
    RaytracingGeometryDesc geometryDesc;
    std::shared_ptr<Resource> rtVertexPositions;
    std::shared_ptr<Resource> rtIndexBuffer;
    std::shared_ptr<Resource> blas;
    uint64_t blas_compacted_size;

    int meshletOffset;

	Mesh() = default;
	~Mesh() = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void PrepareMeshletData();
    void PrepareBLASData(std::shared_ptr<Device> device);
    std::shared_ptr<Resource> CreateBLAS(std::shared_ptr<Device> device, std::shared_ptr<Resource> accelerationStructuresBuffer, uint64_t offset, std::shared_ptr<Resource> scratch);

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