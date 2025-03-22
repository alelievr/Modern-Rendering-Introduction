#pragma once
#include <memory>
#include "Instance/Instance.h"
#include "Material.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include <directx/d3dx12.h>
#include <CommandList/DXCommandList.h>
#include <Resource/DXResource.h>
#include <Device/DXDevice.h>
#include <BindingSetLayout/DXBindingSetLayout.h>

struct IndirectDispatchCommand
{
	unsigned instanceID;
	D3D12_DISPATCH_ARGUMENTS dispatchArgs;
};

class RenderUtils
{
public:
	enum BindingFlags
	{
		TextureList = 1 << 0,
		MeshPool = 1 << 1,
		CameraData = 1 << 2,
		MaterialBuffers = 1 << 3,
		SceneInstances = 1 << 4,
		Sky = 1 << 5,
		All = ~0,
	};

	enum BindingStages
	{
		Mesh = 1 << 0,
		Amplification = 1 << 1,
		Fragment = 1 << 2,
		Compute = 1 << 3,
	};

	struct ComputeProgram
	{
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<Program> program;
		std::shared_ptr<Shader> shader;
	};

	static std::shared_ptr<BindingSetLayout> CreateLayoutSet(std::shared_ptr<Device> device, const Camera& camera, const std::vector<BindKey>& keys, const int flags, const int stages);
	static std::shared_ptr<BindingSet> CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout, const Camera& camera, const std::vector<BindingDesc>& descs, const int flags, const int stages);
	static void UploadBufferData(std::shared_ptr<Device> device, std::shared_ptr<Resource> buffer, const void* data, size_t size);
	static void SetBackgroundColor(GLFWwindow* window, COLORREF color);
	static void UploadTextureData(const std::shared_ptr<Resource>& resource, const std::shared_ptr<Device>& device, uint32_t subresource, const void* data, int width, int height, int channels, int bytePerChannel);
	static ComputeProgram CreateComputePipeline(std::shared_ptr<Device> device, const std::string& shaderPath, const std::string& kernelName, std::shared_ptr<BindingSetLayout> layoutSet);
	static ComPtr<ID3D12CommandSignature> CreateIndirectRootConstantCommandSignature(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layoutSet, bool compute);

	template<typename T>
	static void AllocateVertexBufer(std::shared_ptr<Device> device, const std::vector<T>& data, ViewType viewType, gli::format format, const std::string& name, std::shared_ptr<Resource>& resource, std::shared_ptr<View>& view)
	{
		resource = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(T) * data.size());
		resource->CommitMemory(MemoryType::kDefault);
		RenderUtils::UploadBufferData(device, resource, data.data(), sizeof(T) * data.size());
		resource->SetName(name);

		UploadBufferData(device, resource, data.data(), sizeof(data.front()) * data.size());

		ViewDesc d = {};
		d.view_type = viewType;
		d.dimension = ViewDimension::kBuffer;
		d.buffer_format = format;
		d.structure_stride = sizeof(T);
		d.buffer_size = sizeof(T) * data.size();
		view = device->CreateView(resource, d);
	}
};
