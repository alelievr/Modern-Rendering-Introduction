#pragma once
#include <memory>
#include "Instance/Instance.h"
#include "Material.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

static class RenderUtils
{
public:
	static std::shared_ptr<BindingSetLayout> CreateLayoutSet(std::shared_ptr<Device> device, const Camera& camera, const std::vector<BindKey>& keys);
	static std::shared_ptr<BindingSet> CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout, const Camera& camera, const std::vector<BindingDesc>& descs);
	static void SetBackgroundColor(GLFWwindow* window, COLORREF color);
};
