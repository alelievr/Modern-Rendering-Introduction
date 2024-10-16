#pragma once

#include <string>

enum class PBRTextureType
{
	Albedo,
	Normal,
	Roughness,
	SpecularColor,
	Metalness,
	AmbientOcclusion,
};

class Texture
{
public:
	PBRTextureType type;
	std::string path;

	void LoadTextureData();
	// TODO: commit memory to GPU and binding code
	~Texture() = default;
};