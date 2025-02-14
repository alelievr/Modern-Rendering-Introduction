#pragma once
#include <memory>
#include "Instance/Instance.h"
#include "Material.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

class RenderUtils
{
public:
	static std::shared_ptr<BindingSetLayout> CreateLayoutSet(std::shared_ptr<Device> device, const Camera& camera, const std::vector<BindKey>& keys);
	static void UploadBufferData(std::shared_ptr<Device> device, std::shared_ptr<Resource> buffer, const void* data, size_t size);
	static std::shared_ptr<BindingSet> CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout, const Camera& camera, const std::vector<BindingDesc>& descs);
	static void SetBackgroundColor(GLFWwindow* window, COLORREF color);
	static void UploadTextureData(const std::shared_ptr<Resource>& resource, const std::shared_ptr<Device>& device, uint32_t subresource, const void* data, int width, int height, int channels, int bytePerChannel);

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
