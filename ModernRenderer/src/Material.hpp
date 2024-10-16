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

struct MaterialParameter
{
	std::string name;
	MaterialParameterType type;

	Texture textureValue;
	glm::vec4 float4Value;
	float floatValue;
	int intValue;
};

class Material
{
public:
	std::vector<MaterialParameter> parameters;
	std::string name;

	Material() = default;
	~Material() = default;

	void AddTextureParameter(const Texture& texture)
	{
		parameters.push_back({ texture.path, MaterialParameterType::Texture, texture });
	}
};