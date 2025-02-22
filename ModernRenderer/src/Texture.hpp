#pragma once

#include <string>
#include <memory>
#include <unordered_set>

#include "Instance/Instance.h"

enum class PBRTextureType
{
	BaseColor,
	Normal,
	Roughness,
	SpecularColor,
	Metalness,
	AmbientOcclusion,
};

class Texture
{
private:
	static std::vector<std::shared_ptr<Texture>> textures;
	std::shared_ptr<Texture> instance;

	Texture (const Texture& textyre) = delete;
	Texture& operator=(const Texture& texture) = delete;

public:
	PBRTextureType type;
	std::string path;

	int width;
	int height;
	int channels;
	gli::format format;

	// GPU data
	std::shared_ptr<Resource> resource;
	std::shared_ptr<View> shaderResourceView;

	static std::vector<BindingDesc> textureBufferBindings;
	static std::vector<BindKey> textureBufferBindKeys;

	void LoadTextureData();
	Texture(PBRTextureType type, const std::string& path);

	~Texture();

	static void LoadAllTextures(std::shared_ptr<Device> device);
	static std::shared_ptr<Texture> GetOrCreate(PBRTextureType type, const std::string& path);
};