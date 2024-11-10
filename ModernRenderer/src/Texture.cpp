#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <Device/DXDevice.h>
#include "RenderDoc.hpp"

std::vector<std::shared_ptr<Texture>> Texture::textures;

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

Texture::Texture(PBRTextureType type, const std::string& path)
{
    this->type = type;
	this->path = path;
}

Texture::~Texture()
{
    textures.erase(std::remove(textures.begin(), textures.end(), instance), textures.end());
}

void UploadTextureData(const std::shared_ptr<Resource>& resource, const std::shared_ptr<Device>& device, uint32_t subresource, const void* data, int width, int height, int channels, int bytePerChannel)
{
    std::shared_ptr<CommandQueue> queue = device->GetCommandQueue(CommandListType::kGraphics);
    int numBytes = width * height * channels * bytePerChannel;
    int rowBytes = width * channels * bytePerChannel;

    std::vector<BufferToTextureCopyRegion> regions;
    auto& region = regions.emplace_back();
    region.texture_mip_level = subresource % resource->GetLevelCount();
    region.texture_array_layer = subresource / resource->GetLevelCount();
    region.texture_extent.width = std::max<uint32_t>(1, resource->GetWidth() >> region.texture_mip_level);
    region.texture_extent.height = std::max<uint32_t>(1, resource->GetHeight() >> region.texture_mip_level);
    region.texture_extent.depth = 1;
    region.buffer_row_pitch = rowBytes;
    region.buffer_offset = 0;

    auto upload_resource = device->CreateBuffer(BindFlag::kCopySource, numBytes);
    upload_resource->CommitMemory(MemoryType::kUpload);
    int bufferRowPitch = rowBytes;
    int bufferDepthPitch = bufferRowPitch * height;
    int srcRowPitch = rowBytes;
    int srcDepthPitch = srcRowPitch * height;
    int numRows = height;
    int numSlices = region.texture_extent.depth;
    upload_resource->UpdateUploadBufferWithTextureData(0, bufferRowPitch, bufferDepthPitch, data, srcRowPitch, srcDepthPitch,
        numRows, numSlices);

    std::shared_ptr<CommandList> cmd = device->CreateCommandList(CommandListType::kGraphics);

    RenderDoc::StartCaptureImmediately();

    cmd->Reset();
    cmd->BeginEvent("UploadTextureData");
    // TODO execute commands as the tmp buffer will be destroyed when the function exits
    cmd->ResourceBarrier({ { resource, ResourceState::kCommon, ResourceState::kCopyDest } });
    cmd->ResourceBarrier({ { upload_resource, ResourceState::kCommon, ResourceState::kCopySource } });
    //cmd->ResourceBarrier({ barrierDesc });
    //ImageBarrier(resource, region.texture_mip_level, 1, region.texture_array_layer, 1, ResourceState::kCopyDest);
    cmd->CopyBufferToTexture(upload_resource, resource, regions);
    cmd->EndEvent();
    cmd->Close();
    queue->ExecuteCommandLists({ cmd });

    std::shared_ptr<Fence> fence = device->CreateFence(0);
    queue->Signal(fence, 1);
    queue->Wait(fence, 1);
    fence->Wait(1);

    RenderDoc::EndFrameCapture();

    auto d = (DXDevice*)device.get();
    auto a = d->GetDevice();
    auto h = a->GetDeviceRemovedReason();
    printf("%i\n", h);
}

void Texture::LoadAllTextures(std::shared_ptr<Device> device)
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

        // TODO: support HDR textures
        UploadTextureData(texture->resource, device, 0, image, width, height, channels, 1);

        // TODO: fenerate mipmaps
        //cmd->GenerateMipmaps(res);
    }

    // Once all textures are loaded, we can init the bindless arrays

    std::map<uint32_t, std::shared_ptr<View>> views;
    int index = 0;
    for (auto& texture : textures)
    {
        if (texture->type != PBRTextureType::Albedo)
			continue;

        ViewDesc view_desc = {};
        view_desc.bindless = true;
        view_desc.dimension = ViewDimension::kTexture2D;
        view_desc.view_type = ViewType::kTexture;
        texture->shaderResourceView = device->CreateView(texture->resource, view_desc);
    }
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
