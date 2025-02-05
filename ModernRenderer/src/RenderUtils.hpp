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
	static std::shared_ptr<BindingSet> CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout, const Camera& camera, const std::vector<BindingDesc>& descs);
	static void SetBackgroundColor(GLFWwindow* window, COLORREF color);

	template<typename T>
	static void AllocateVertexBufer(std::shared_ptr<Device> device, const std::vector<T>& data, ViewType viewType, gli::format format, const std::string& name, std::shared_ptr<Resource>& resource, std::shared_ptr<View>& view)
	{
		resource = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(T) * data.size());
		resource->CommitMemory(MemoryType::kUpload); // TODO: use the default memory type
		resource->UpdateUploadBuffer(0, data.data(), sizeof(data.front()) * data.size());
		resource->SetName(name);

		ViewDesc d = {};
		d.view_type = viewType;
		d.dimension = ViewDimension::kBuffer;
		d.buffer_format = format;
		d.structure_stride = sizeof(T);
		d.buffer_size = sizeof(T) * data.size();
		view = device->CreateView(resource, d);
	}
};
