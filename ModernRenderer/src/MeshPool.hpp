#pragma once

#include <vector>
#include "Instance/Instance.h"
#include "meshoptimizer.h"
#include "Mesh.hpp"
#include <unordered_set>

class MeshPool
{
public:
    static std::unordered_map<std::shared_ptr<Mesh>, unsigned> meshes;

    static std::vector<meshopt_Meshlet> meshlets;
    static std::vector<uint32_t> meshletIndices;
    static std::vector<uint8_t> meshletTriangles;
    static std::vector<Mesh::Vertex> vertices;

    static std::shared_ptr<Resource> vertexPool;
    static std::shared_ptr<Resource> meshletIndicesPool;
    static std::shared_ptr<Resource> meshletTrianglesPool;
    static std::shared_ptr<Resource> meshletsPool;

    static std::shared_ptr<View> vertexPoolView;
    static std::shared_ptr<View> meshletIndicesPoolView;
    static std::shared_ptr<View> meshletTrianglesPoolView;
    static std::shared_ptr<View> meshletsPoolView;

    static std::vector<BindingDesc> bindingDescs;
    static std::vector<BindKey> bindKeys;

    static unsigned PushNewMesh(std::shared_ptr<Mesh> mesh);
    static void AllocateMeshPoolBuffers(std::shared_ptr<Device> device);
};