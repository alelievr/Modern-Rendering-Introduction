#pragma once

#include <fstream>
#include <set>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include "Mesh.hpp"
#include "Model.hpp"
#include "Texture.hpp"

class ModelImporter
{
private:
    std::string path;
    std::string directory;
    Assimp::Importer m_import;
    Model model = {};

    void LoadModel(int flags);
    std::string SplitFilename(const std::string& str);
    void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4 parentTransformation);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4 transform);
    void FindSimilarTextures(const std::string& mat_name, std::vector<std::shared_ptr<Texture>>& textures);
    void LoadMaterialTextures(aiMaterial* mat, aiTextureType aitype, PBRTextureType type, std::vector<std::shared_ptr<Texture>>& textures);

public:
    ModelImporter(const std::string& path, int flags);

    Model& GetModel() { return model; }
};