#pragma once

#include <string>
#include <vector>
#include "Texture.hpp"
#include <glm/glm.hpp>

enum class MaterialParameterType
{
	Float,
	Float4,
	Matrix4x4,
	Texture,
	Int,
};

// Keep in sync with MaterialData in Common.hlsl
struct GPUMaterial
{
	// Base layer
	// float baseWeight;
	glm::vec3 baseColor;
	int baseColorTextureIndex;
	float metalness;
	int metalnessTextureIndex;
	float diffuseRoughness;
	int diffuseRoughnessTextureIndex;

	// Specular layer
	// TODO
	//float specularWeight;
	//glm::vec3 specularColor;
	//int specularColorTextureIndex;
	//float specularRoughness;
	//float specularRoughnessAnisotropy;
	//float specularIOR;

	// TODO: Transmission, Coat, Fuzz, Emission and Thin-Film

	// Geometry
	int normalTextureIndex;
	int ambientOcclusionTextureIndex;
	//float opacity; // TODO: transparency
	// Coat normal?

	// Extra padding to ensure the struct is aligned on 16 bytes
	int padding0;
	int padding1;
};

class Material
{
public:
	static std::vector<std::shared_ptr<Material>> instances;
	static std::vector<GPUMaterial> materialBuffer;
	static std::shared_ptr<Resource> materialConstantBuffer;
	static std::shared_ptr<View> materialConstantBufferView;
	static BindingDesc materialBufferBinding;
	static BindKey materialBufferBindKey;

	// Base data
	bool specularWorkflow = false;
	std::shared_ptr<Texture> baseColorTexture = nullptr;
	glm::vec3 baseColor = glm::vec3(0.8, 0.8, 0.8);
	std::shared_ptr<Texture> metalnessTexture = nullptr;
	float metalness = 0;
	std::shared_ptr<Texture> roughnessTexture = nullptr;
	float roughness = 0.5f;
	
	// TODO: Transmission, SSS, Coat, Fuzz, Emission and Thin-Film

	// Specular workflow data
	std::shared_ptr<Texture> specularColorTexture = nullptr;
	glm::vec3 specularColor = glm::vec3(0.8, 0.8, 0.8);

	// Geometry data
	std::shared_ptr<Texture> normalTexture = nullptr;
	std::shared_ptr<Texture> ambientOcclusion = nullptr;
	// Heightmap, etc.

	std::string name;
	int materialIndex;

	Material() = default;
	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	~Material();

	static std::shared_ptr<Material> CreateMaterial();
	void AddTextureParameter(std::shared_ptr<Texture> texture);
	static void AllocateMaterialBuffers(std::shared_ptr<Device> device);
	static bool Compare(const std::shared_ptr<Material>& a, const std::shared_ptr<Material>& b);
};
