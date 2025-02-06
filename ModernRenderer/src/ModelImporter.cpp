// Forked from https://github.com/andrejnau/FlyWrapper/blob/354db8633b210b2a4bc259e8b05aaf6460ca033d/src/Modules/Geometry/ModelLoader.cpp

#include "ModelImporter.hpp"
#include "Material.hpp"
#include "meshoptimizer.h"
#include <corecrt_io.h>

glm::vec3 AiVector3DToVec3(const aiVector3D& x)
{
    return glm::vec3(x.x, x.y, x.z);
}

ModelImporter::ModelImporter(const std::string& path, int flags)
    : path(path)
    , directory(SplitFilename(path))
{
    m_import.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
    LoadModel(flags | aiProcess_ConvertToLeftHanded);
}

std::string ModelImporter::SplitFilename(const std::string& str)
{
    return str.substr(0, str.find_last_of("/"));
}

void ModelImporter::LoadModel(int flags)
{
    if (_access(path.c_str(), 4) == -1)
    {
        printf("Can't open 3D model at %s\n", path.c_str());
        return;
    }

    const aiScene* scene = m_import.ReadFile(path, flags);
    if (scene == nullptr)
    {
        printf("Can't load 3D model at %s\n", path.c_str());
        return;
    }
    assert(scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode);

    if (path.ends_with(".fbx"))
    {
        scene->mMetaData->Set("UnitScaleFactor", 0.01f);
    }

    ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

glm::mat4 AiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = from->a1; to[0][1] = from->b1;  to[0][2] = from->c1; to[0][3] = from->d1;
    to[1][0] = from->a2; to[1][1] = from->b2;  to[1][2] = from->c2; to[1][3] = from->d2;
    to[2][0] = from->a3; to[2][1] = from->b3;  to[2][2] = from->c3; to[2][3] = from->d3;
    to[3][0] = from->a4; to[3][1] = from->b4;  to[3][2] = from->c4; to[3][3] = from->d4;

    return to;
}

void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4 parentTransformation)
{
    glm::mat4 transformation = AiMatrix4x4ToGlm(&node->mTransformation);
    glm::mat4 globalTransformation = transformation * parentTransformation;

    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, globalTransformation);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], scene, globalTransformation);
    }

    // Deduplicate materials
    std::vector<std::shared_ptr<Material>> uniqueMaterials;
    for (auto& part : model.parts)
	{
        for (auto& m : uniqueMaterials)
            if (Material::Compare(m, part.material))
			{
				part.material = m;
				break;
			}

		uniqueMaterials.push_back(part.material);
	}

    Material::instances.insert(Material::instances.end(), uniqueMaterials.begin(), uniqueMaterials.end());
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

void ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4 transform)
{
    if (SkipMesh(mesh, scene))
        return;

    float scale = 1;
    scene->mMetaData->Get("UnitScaleFactor", scale);

    Mesh currentMesh = {};
    currentMesh.name = mesh->mName.C_Str();
    std::shared_ptr<Material> currentMaterial = Material::CreateMaterial();
    // Walk through each of the mesh's vertices
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texcoord;
            glm::vec3 tangent;
            glm::vec3 bitangent;
        } vertex;

        if (mesh->HasPositions())
        {
            vertex.position.x = mesh->mVertices[i].x * scale;
            vertex.position.y = mesh->mVertices[i].y * scale;
            vertex.position.z = mesh->mVertices[i].z * scale;

            vertex.position = transform * glm::vec4(vertex.position, 1);
        }

        if (mesh->HasNormals())
        {
            vertex.normal = AiVector3DToVec3(mesh->mNormals[i]);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = AiVector3DToVec3(mesh->mTangents[i]);
            vertex.bitangent = AiVector3DToVec3(mesh->mBitangents[i]);

            if (glm::dot(glm::cross(vertex.normal, vertex.tangent), vertex.bitangent) < 0.0f)
                vertex.tangent *= -1.0f;
        }

        // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        if (mesh->HasTextureCoords(0))
            vertex.texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else
            vertex.texcoord = glm::vec2(0.0f, 0.0f);

        currentMesh.positions.push_back(vertex.position);
        currentMesh.normals.push_back(vertex.normal);
        currentMesh.texcoords.push_back(vertex.texcoord);
        currentMesh.tangents.push_back(vertex.tangent);
    }
    // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex
    // indices.
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        // Retrieve all indices of the face and store them in the indices vector
        for (uint32_t j = 0; j < face.mNumIndices; ++j)
            currentMesh.indices.push_back(face.mIndices[j]);
    }

    // Process materials
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiString name;
        if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
        {
            currentMaterial->name = name.C_Str();
        }

        std::vector<std::shared_ptr<Texture>> textures;
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

        FindSimilarTextures(currentMaterial->name, textures);

        for (const auto& texture : textures)
            currentMaterial->AddTextureParameter(texture);
    }

    model.parts.push_back({ currentMesh, currentMaterial } );
}

void ModelImporter::FindSimilarTextures(const std::string& mat_name, std::vector<std::shared_ptr<Texture>>& textures)
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
        used.insert(cur_texture->type);
    }

    if (!used.count(PBRTextureType::Albedo)) {
        for (auto& ext : { ".dds", ".png", ".jpg" }) {
            std::string cur_path = directory + "/textures/" + mat_name + "_albedo" + ext;
            if (std::ifstream(cur_path).good()) {
                textures.push_back(Texture::GetOrCreate(PBRTextureType::Albedo, cur_path));
            }
            cur_path = directory + "/" + "albedo" + ext;
            if (std::ifstream(cur_path).good()) {
                textures.push_back(Texture::GetOrCreate(PBRTextureType::Albedo, cur_path));
            }
        }
    }

    std::vector<std::shared_ptr<Texture>> added_textures;
    for (auto& from_type : texture_types) {
        for (auto& cur_texture : textures) {
            std::string path = cur_texture->path;

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

                added_textures.push_back(Texture::GetOrCreate(to_type.second, cur_path));
                used.insert(to_type.second);
            }
        }
    }

    textures.insert(textures.end(), added_textures.begin(), added_textures.end());
}

void ModelImporter::LoadMaterialTextures(aiMaterial* mat,
    aiTextureType aitype,
    PBRTextureType type,
    std::vector<std::shared_ptr<Texture>>& textures)
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

        textures.push_back(Texture::GetOrCreate(type, texture_path));
    }
}