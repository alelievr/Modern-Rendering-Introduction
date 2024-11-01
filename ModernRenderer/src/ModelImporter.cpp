// Forked from https://github.com/andrejnau/FlyWrapper/blob/354db8633b210b2a4bc259e8b05aaf6460ca033d/src/Modules/Geometry/ModelLoader.cpp

#include "ModelImporter.hpp"
#include <glm/glm.hpp>
#include "Material.hpp"

glm::vec3 aiVector3DToVec3(const aiVector3D& x)
{
    return glm::vec3(x.x, x.y, x.z);
}

ModelImporter::ModelImporter(const std::string& path, int flags)
    : path(path)
    , directory(SplitFilename(path))
{
    m_import.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
    LoadModel(flags);
}

std::string ModelImporter::SplitFilename(const std::string& str)
{
    return str.substr(0, str.find_last_of("/"));
}

void ModelImporter::LoadModel(int flags)
{
    const aiScene* scene = m_import.ReadFile(
        path, flags & (aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_Triangulate |
            aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_ValidateDataStructure));
    assert(scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode);

    if (path.ends_with(".fbx"))
    {
        scene->mMetaData->Set("UnitScaleFactor", 0.01f);
    }

    ProcessNode(scene->mRootNode, scene);
}

void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], scene);
    }
}

inline glm::vec4 aiColor4DToVec4(const aiColor4D& x)
{
    return glm::vec4(x.r, x.g, x.b, x.a);
}

bool SkipMesh(aiMesh* mesh, const aiScene* scene)
{
    if (mesh->mMaterialIndex >= scene->mNumMaterials) {
        return false;
    }
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    aiString name;
    if (!mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
        return false;
    }
    static std::set<std::string> q = { "16___Default", "Ground_SG" };
    return q.count(std::string(name.C_Str()));
}

void ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    if (SkipMesh(mesh, scene))
        return;

    float scale;
    scene->mMetaData->Get("UnitScaleFactor", scale);

    Mesh currentMesh = {};
    Material currentMaterial;
    // Walk through each of the mesh's vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texcoord;
            glm::vec3 tangent;
            glm::vec3 bitangent;
        } vertex;

        if (mesh->HasPositions()) {
            vertex.position.x = mesh->mVertices[i].x * scale;
            vertex.position.y = mesh->mVertices[i].y * scale;
            vertex.position.z = mesh->mVertices[i].z * scale;
        }

        if (mesh->HasNormals()) {
            vertex.normal = aiVector3DToVec3(mesh->mNormals[i]);
        }

        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = aiVector3DToVec3(mesh->mTangents[i]);
            vertex.bitangent = aiVector3DToVec3(mesh->mBitangents[i]);

            if (glm::dot(glm::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f) {
                vertex.tangent *= -1.0f;
            }
        }

        // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        if (mesh->HasTextureCoords(0)) {
            vertex.texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.texcoord = glm::vec2(0.0f, 0.0f);
        }

        currentMesh.positions.push_back(vertex.position);
        currentMesh.normals.push_back(vertex.normal);
        currentMesh.texcoords.push_back(vertex.texcoord);
        currentMesh.tangents.push_back(vertex.tangent);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex
    // indices.
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            currentMesh.indices.push_back(face.mIndices[j]);
        }
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiString name;
        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
            currentMaterial.name = name.C_Str();
        }

        std::vector<Texture> textures;
        // map_Kd
        LoadMaterialTextures(mat, aiTextureType_DIFFUSE, PBRTextureType::Albedo, textures);
        // map_bump
        LoadMaterialTextures(mat, aiTextureType_NORMALS, PBRTextureType::Normal, textures);
        // map_Ns
        LoadMaterialTextures(mat, aiTextureType_SHININESS, PBRTextureType::Roughness, textures);
        // map_Ks
        LoadMaterialTextures(mat, aiTextureType_SPECULAR, PBRTextureType::Metalness, textures);
        // map_d
        //LoadMaterialTextures(mat, aiTextureType_OPACITY, PBRTextureType::kOpacity, textures);

        FindSimilarTextures(currentMaterial.name, textures);

        auto comparator = [&](const Texture& lhs, const Texture& rhs) {
            return std::tie(lhs.type, lhs.path) < std::tie(rhs.type, rhs.path);
            };

        std::set<Texture, decltype(comparator)> unique_textures(comparator);

        for (Texture& texture : textures) {
            unique_textures.insert(texture);
        }

        for (const Texture& texture : unique_textures) {
            currentMaterial.AddTextureParameter(texture);
        }
    }

    model.parts.push_back({ currentMesh, currentMaterial } );
}

void ModelImporter::FindSimilarTextures(const std::string& mat_name, std::vector<Texture>& textures)
{
    static std::pair<std::string, PBRTextureType> texture_types[] = {
        { "albedo", PBRTextureType::Albedo },        { "_albedo", PBRTextureType::Albedo },
        { "_Albedo", PBRTextureType::Albedo },       { "_color", PBRTextureType::Albedo },
        { "_diff", PBRTextureType::Albedo },         { "_diffuse", PBRTextureType::Albedo },
        { "_BaseColor", PBRTextureType::Albedo },    { "_nmap", PBRTextureType::Normal },
        { "normal", PBRTextureType::Normal },        { "_normal", PBRTextureType::Normal },
        { "_Normal", PBRTextureType::Normal },       { "_rough", PBRTextureType::Roughness },
        { "roughness", PBRTextureType::Roughness },  { "_roughness", PBRTextureType::Roughness },
        { "_Roughness", PBRTextureType::Roughness },/* {"_gloss", PBRTextureType::kGlossiness},*/
        { "_metalness", PBRTextureType::Metalness }, { "_metallic", PBRTextureType::Metalness },
        { "metallic", PBRTextureType::Metalness },   { "_Metallic", PBRTextureType::Metalness },
        { "_ao", PBRTextureType::AmbientOcclusion }, { "ao", PBRTextureType::AmbientOcclusion },
    //  { "_mask", PBRTextureType::kOpacity },        { "_opacity", PBRTextureType::kOpacity },
    };
    std::set<PBRTextureType> used;
    for (auto& cur_texture : textures) {
        used.insert(cur_texture.type);
    }

    if (!used.count(PBRTextureType::Albedo)) {
        for (auto& ext : { ".dds", ".png", ".jpg" }) {
            std::string cur_path = directory + "/textures/" + mat_name + "_albedo" + ext;
            if (std::ifstream(cur_path).good()) {
                textures.push_back(Texture{ PBRTextureType::Albedo, cur_path });
            }
            cur_path = directory + "/" + "albedo" + ext;
            if (std::ifstream(cur_path).good()) {
                textures.push_back({ PBRTextureType::Albedo, cur_path });
            }
        }
    }

    std::vector<Texture> added_textures;
    for (auto& from_type : texture_types) {
        for (auto& cur_texture : textures) {
            std::string path = cur_texture.path;

            size_t loc = path.find(from_type.first);
            if (loc == std::string::npos) {
                continue;
            }

            for (auto& to_type : texture_types) {
                if (used.count(to_type.second)) {
                    continue;
                }
                std::string cur_path = path;
                cur_path.replace(loc, from_type.first.size(), to_type.first);
                if (!std::ifstream(cur_path).good()) {
                    continue;
                }

                Texture texture;
                texture.type = to_type.second;
                texture.path = cur_path;
                added_textures.push_back(texture);
                used.insert(to_type.second);
            }
        }
    }

    textures.insert(textures.end(), added_textures.begin(), added_textures.end());
}

void ModelImporter::LoadMaterialTextures(aiMaterial* mat,
    aiTextureType aitype,
    PBRTextureType type,
    std::vector<Texture>& textures)
{
    for (uint32_t i = 0; i < mat->GetTextureCount(aitype); ++i) {
        aiString texture_name;
        mat->GetTexture(aitype, i, &texture_name);
        std::string texture_path = directory + "/" + texture_name.C_Str();
        std::replace(texture_path.begin(), texture_path.end(), '\\', '/');
        if (!std::ifstream(texture_path).good()) {
            texture_path = texture_path.substr(0, texture_path.rfind('.')) + ".dds";
            if (!std::ifstream(texture_path).good()) {
                continue;
            }
        }

        Texture texture;
        texture.type = type;
        texture.path = texture_path;
        textures.push_back(texture);
    }
}