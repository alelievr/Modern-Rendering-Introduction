#pragma once

#include <fstream>
#include <set>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Mesh.hpp"
#include "Model.hpp"
#include "Texture.hpp"

class ModelImporter
{
private:
    std::string path;
    std::string directory;
    Assimp::Importer m_import;
    std::vector<Mesh> m_meshes;
    Model model = {};

    void LoadModel(int flags);
    std::string SplitFilename(const std::string& str);
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene);
    void FindSimilarTextures(const std::string& mat_name, std::vector<Texture>& textures);
    void LoadMaterialTextures(aiMaterial* mat, aiTextureType aitype, PBRTextureType type, std::vector<Texture>& textures);

public:
    ModelImporter(const std::string& path, int flags);

    Model& GetModel() { return model; }
};