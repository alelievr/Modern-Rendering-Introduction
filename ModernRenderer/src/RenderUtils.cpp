#include "RenderUtils.hpp"
#include "Mesh.hpp"
#include "MeshPool.hpp"
#include "Scene.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include <algorithm>

static ShaderType GetShaderTypeFromStage(int stage)
{
	switch (stage)
	{
	case RenderUtils::Mesh:
		return ShaderType::kMesh;
	case RenderUtils::Amplification:
		return ShaderType::kAmplification;
	case RenderUtils::Fragment:
		return ShaderType::kPixel;
	case RenderUtils::Compute:
		return ShaderType::kCompute;
	default:
		throw std::exception("Unknown shader stage");
	}
}

std::shared_ptr<BindingSetLayout> RenderUtils::CreateLayoutSet(std::shared_ptr<Device> device, const Camera& camera, const std::vector<BindKey>& keys, const int flags, const int stages)
{
	// Add bindless textures and material datas
	std::vector<BindKey> allBindings = keys;
	
	if (flags & CameraData)
	{
		allBindings.emplace_back(camera.cameraDataKeyVertex);
	}

	if (flags & MaterialBuffers)
		allBindings.emplace_back(Material::materialBufferBindKey);

	if (flags & MeshPool)
		allBindings.insert(allBindings.end(), MeshPool::bindKeys.begin(), MeshPool::bindKeys.end());
	
	if (flags & SceneInstances)
		allBindings.insert(allBindings.end(), Scene::bindKeys.begin(), Scene::bindKeys.end());

	if (flags & TextureList)
	{
		for (const auto& textureKeys : Texture::textureBufferBindKeys)
			allBindings.emplace_back(textureKeys);
	}

	std::vector<BindKey> allBindingsWithStages;
	for (auto& key : allBindings)
	{
		int stageMask = stages;
		while (stageMask != 0)
		{
			BindingStages stage = (BindingStages)(1 << _tzcnt_u32(stageMask));
			stageMask &= stageMask - 1;
			key.shader_type = GetShaderTypeFromStage(stage);
			allBindingsWithStages.push_back(key);
		}
	}

	return device->CreateBindingSetLayout(allBindingsWithStages);
}

void RenderUtils::UploadBufferData(std::shared_ptr<Device> device, std::shared_ptr<Resource> buffer, const void* data, size_t size)
{
	std::shared_ptr<CommandQueue> queue = device->GetCommandQueue(CommandListType::kGraphics);
	uint64_t fenceValue = 0;
	std::shared_ptr<Fence> fence = device->CreateFence(fenceValue);

	std::shared_ptr<Resource> uploadBuffer =
		device->CreateBuffer(BindFlag::kCopySource, buffer->GetWidth());
	uploadBuffer->CommitMemory(MemoryType::kUpload);
	uploadBuffer->SetName("Upload Tmp Buffer");
	uploadBuffer->UpdateUploadBuffer(0, data, size);

	std::shared_ptr<CommandList> cmd = device->CreateCommandList(CommandListType::kGraphics);
	cmd->ResourceBarrier({ { buffer, ResourceState::kCommon, ResourceState::kCopyDest } });
	cmd->CopyBuffer(uploadBuffer, buffer, { { 0, 0, buffer->GetWidth() } });
	cmd->ResourceBarrier({ { buffer, ResourceState::kCopyDest, ResourceState::kCommon } });
	cmd->Close();

	queue->ExecuteCommandLists({ cmd });
	queue->Signal(fence, ++fenceValue);
	fence->Wait(fenceValue);
}

std::shared_ptr<BindingSet> RenderUtils::CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout,
	const Camera& camera, const std::vector<BindingDesc>& descs, const int flags, const int stages)
{
	std::vector<BindingDesc> allBindings = descs;

	if (flags & CameraData)
		allBindings.emplace_back(camera.cameraDataDescVertex);
	
	if (flags & MaterialBuffers)
		allBindings.emplace_back(Material::materialBufferBinding);
	
	if (flags & MeshPool)
		allBindings.insert(allBindings.end(), MeshPool::bindingDescs.begin(), MeshPool::bindingDescs.end());

	if (flags & SceneInstances)
		allBindings.insert(allBindings.end(), Scene::bindingDescs.begin(), Scene::bindingDescs.end());

	if (flags & TextureList)
	{
		for (const auto& textureBindings : Texture::textureBufferBindings)
			allBindings.emplace_back(textureBindings);
	}

	std::vector<BindingDesc> allBindingsWithStages;
	for (auto& desc : allBindings)
	{
		int stageMask = stages;
		while (stageMask != 0)
		{
			BindingStages stage = (BindingStages)(1 << _tzcnt_u32(stageMask));
			stageMask &= stageMask - 1;
			desc.bind_key.shader_type = GetShaderTypeFromStage(stage);
			allBindingsWithStages.push_back(desc);
		}
	}

	auto set = device->CreateBindingSet(layout);
	set->WriteBindings(allBindingsWithStages);

	return set;
}

void RenderUtils::SetBackgroundColor(GLFWwindow* window, COLORREF color)
{
	HWND hwnd = glfwGetWin32Window(window);
	HDC hdc = GetDC(hwnd);

	RECT rect;
	GetClientRect(hwnd, &rect);

	HBRUSH brush = CreateSolidBrush(color);
	FillRect(hdc, &rect, brush);

	DeleteObject(brush);
	ReleaseDC(hwnd, hdc);
}

void RenderUtils::UploadTextureData(const std::shared_ptr<Resource>& resource, const std::shared_ptr<Device>& device, uint32_t subresource, const void* data, int width, int height, int channels, int bytePerChannel)
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
	upload_resource->SetName("Tmp Texture Upload Buffer");
	int bufferRowPitch = rowBytes;
	int bufferDepthPitch = bufferRowPitch * height;
	int srcRowPitch = rowBytes;
	int srcDepthPitch = srcRowPitch * height;
	int numRows = height;
	int numSlices = region.texture_extent.depth;
	upload_resource->UpdateUploadBufferWithTextureData(0, bufferRowPitch, bufferDepthPitch, data, srcRowPitch, srcDepthPitch,
		numRows, numSlices);

	std::shared_ptr<CommandList> cmd = device->CreateCommandList(CommandListType::kGraphics);

	cmd->Reset();
	cmd->BeginEvent("UploadTextureData");

	cmd->ResourceBarrier({ { resource, ResourceState::kCommon, ResourceState::kCopyDest } });

	cmd->CopyBufferToTexture(upload_resource, resource, regions);

	cmd->ResourceBarrier({ { resource, ResourceState::kCopyDest, ResourceState::kCommon } });

	cmd->EndEvent();
	cmd->Close();
	queue->ExecuteCommandLists({ cmd });

	std::shared_ptr<Fence> fence = device->CreateFence(0);
	queue->Signal(fence, 1);
	queue->Wait(fence, 1);
	fence->Wait(1);
}
