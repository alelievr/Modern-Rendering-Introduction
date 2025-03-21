#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <Device/DXDevice.h>
#include "RenderDoc.hpp"
#include <filesystem>
#include "RenderUtils.hpp"

std::vector<std::shared_ptr<Texture>> Texture::textures;
std::vector<BindingDesc> Texture::textureBufferBindings = {};
std::vector<BindKey> Texture::textureBufferBindKeys = {};

void Texture::LoadTextureData()
{
    int width, height, channels;
    unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (image)
    {
        // Process the loaded image data
        // ...

        // Free the image data
        stbi_image_free(image);
    }
    else
    {
       	// Handle error
    }
}

void Texture::LoadAndUpload3DTextureData(std::shared_ptr<Device> device)
{
    // STBN has 64 depth slices
    for (int i = 0; i < 64; i++)
    {
        std::string path = path + "_" + std::to_string(i) + ".png";

        int width, height, channels;
	    unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
        
        gli::format format;
        if (channels == 1)
            format = gli::FORMAT_R8_SNORM_PACK8;
        else if (channels == 2)
            format = gli::FORMAT_RG8_SNORM_PACK8;
		else if (channels == 3)
			format = gli::FORMAT_RGB8_SNORM_PACK8;
		else if (channels == 4)
			format = gli::FORMAT_RGBA8_SNORM_PACK8;
		else
			throw std::exception("Unsupported number of channels");

        if (!resource)
        {
            resource = device->CreateTexture(
                TextureType::k3D,
                BindFlag::kShaderResource | BindFlag::kCopyDest,
                format,
                1, width, height, 64, 1
            );
            resource->CommitMemory(MemoryType::kDefault);
	        resource->SetName(path);
        }
        RenderUtils::UploadTextureData(resource, device, i, image, width, height, channels, 1);
        stbi_image_free(image);
    }
}

Texture::Texture(PBRTextureType type, const std::string& path)
{
    this->type = type;
	this->path = path;
}

Texture::~Texture()
{
    textures.erase(std::remove(textures.begin(), textures.end(), instance), textures.end());
}

void Texture::LoadAllMaterialTextures(std::shared_ptr<Device> device)
{
    for (auto& texture : textures)
    {
        std::string path = texture->path;
        int width, height, channels;
        // TODO: handle textures with less than 4 channels
        unsigned char* image = stbi_load(path.c_str(), &width, &height, NULL, STBI_rgb_alpha);
        channels = 4;

        gli::format format = gli::FORMAT_RGBA8_UNORM_PACK8;

        // compute mip levels
        int mipCount = std::floor(std::log2(std::max(width, height))) + 1;
        texture->resource = device->CreateTexture(
            TextureType::k2D,
            BindFlag::kShaderResource | BindFlag::kCopyDest,
            format,
            1, width, height, 1, mipCount
        );
        texture->resource->CommitMemory(MemoryType::kDefault);

        // Set the file name to the resource for debugging
        std::filesystem::path p(path);
        texture->resource->SetName(p.filename().string());

        // TODO: support HDR textures
        RenderUtils::UploadTextureData(texture->resource, device, 0, image, width, height, channels, 1);

        // TODO: fenerate mipmaps
        //cmd->GenerateMipmaps(res);
    }

    // Once all textures are loaded, we can init the bindless arrays
    BindKey textureKey = { ShaderType::kPixel, ViewType::kTexture, 0, 1, UINT32_MAX, UINT32_MAX };
    textureBufferBindKeys.emplace_back(textureKey);

    std::map<uint32_t, std::shared_ptr<View>> views;
    int index = 0;
    std::vector<BindKey> bindKeys;
    for (auto& texture : textures)
    {
        ViewDesc viewDesc = {};
        viewDesc.bindless = true;
        viewDesc.dimension = ViewDimension::kTexture2D;
        viewDesc.view_type = ViewType::kTexture;
        texture->shaderResourceView = device->CreateView(texture->resource, viewDesc);
        index++;
    }

    // Create samplers
    BindKey samplerLinearClampKey = { ShaderType::kPixel, ViewType::kSampler, 0, 3, 1, UINT32_MAX };
    textureBufferBindKeys.emplace_back(samplerLinearClampKey);
    BindKey sampleLinearRepeatKey = { ShaderType::kPixel, ViewType::kSampler, 1, 3, 1, UINT32_MAX };
    textureBufferBindKeys.emplace_back(sampleLinearRepeatKey);

    ViewDesc sampler_view_desc = {};
    sampler_view_desc.view_type = ViewType::kSampler;

    auto linearClampSampler = device->CreateSampler({
        SamplerFilter::kMinMagMipLinear,
        SamplerTextureAddressMode::kClamp,
        SamplerComparisonFunc::kNever,
    });
    auto linearClampSamplerView = device->CreateView(linearClampSampler, sampler_view_desc);
    textureBufferBindings.emplace_back(BindingDesc{ samplerLinearClampKey, linearClampSamplerView });

    auto linearRepeatSampler = device->CreateSampler({
        SamplerFilter::kMinMagMipLinear,
        SamplerTextureAddressMode::kWrap,
        SamplerComparisonFunc::kNever,
    });
    auto linearRepeatSamplerView = device->CreateView(linearRepeatSampler, sampler_view_desc);
    textureBufferBindings.emplace_back(BindingDesc{ sampleLinearRepeatKey, linearRepeatSamplerView });
}

std::shared_ptr<Texture> Texture::GetOrCreate(PBRTextureType type, const std::string& path)
{
    // If texture type and path already exists, return the texture
    for (auto& texture : textures)
	{
		if (texture->type == type && texture->path == path)
			return texture;
	}

    auto sharedTexture = std::make_shared<Texture>(type, path);
    sharedTexture->instance = sharedTexture;
    textures.push_back(sharedTexture);
    return sharedTexture;
}

std::shared_ptr<Texture> Texture::Create3D(std::shared_ptr<Device> device, const std::string& path)
{
    auto t = std::make_shared<Texture>(0, path);

    t->LoadAndUpload3DTextureData(device);

    return t;
}
