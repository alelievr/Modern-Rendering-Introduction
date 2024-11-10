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

struct GPUMaterial
{
	int albedoTextureIndex;
};

struct MaterialParameter
{
	std::string name;
	MaterialParameterType type;

	std::shared_ptr<Texture> textureValue;
	glm::vec4 float4Value;
	float floatValue;
	int intValue;
};

class Material
{
public:
	static std::vector<std::shared_ptr<Material>> instances;
	static std::vector<GPUMaterial> materialBuffer;
	static std::shared_ptr<Resource> materialConstantBuffer;
	static std::shared_ptr<View> materialConstantBufferView;

	std::vector<MaterialParameter> parameters;
	std::string name;
	int materialIndex;

	Material() = default;
	Material(const Material&) = delete;
	Material& operator=(const Material&) = delete;
	~Material() = default;

	static std::shared_ptr<Material> CreateMaterial();
	void AddTextureParameter(std::shared_ptr<Texture> texture);
	static void AllocateMaterialBuffers(std::shared_ptr<Device> device);
};