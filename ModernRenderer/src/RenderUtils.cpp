#include "RenderUtils.hpp"
#include "Mesh.hpp"
#include "MeshPool.hpp"
#include "Scene.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

std::shared_ptr<BindingSetLayout> RenderUtils::CreateLayoutSet(std::shared_ptr<Device> device, const Camera& camera, const std::vector<BindKey>& keys)
{
	// Add bindless textures and material datas
	std::vector<BindKey> allBindings = keys;
	allBindings.emplace_back(camera.cameraDataKeyVertex);
	allBindings.emplace_back(camera.cameraDataKeyFragment);
	allBindings.emplace_back(camera.cameraDataKeyMesh);
	allBindings.emplace_back(camera.cameraDataKeyAmplification);
	allBindings.emplace_back(Material::materialBufferBindKey);
	allBindings.insert(allBindings.end(), MeshPool::bindKeys.begin(), MeshPool::bindKeys.end());
	allBindings.insert(allBindings.end(), Scene::bindKeys.begin(), Scene::bindKeys.end());
	for (const auto& textureKeys : Texture::textureBufferBindKeys)
		allBindings.emplace_back(textureKeys);
	return device->CreateBindingSetLayout(allBindings);
}

std::shared_ptr<BindingSet> RenderUtils::CreateBindingSet(std::shared_ptr<Device> device, std::shared_ptr<BindingSetLayout> layout, const Camera& camera, const std::vector<BindingDesc>& descs)
{
	std::vector<BindingDesc> allBindings = descs;

	allBindings.emplace_back(camera.cameraDataDescVertex);
	allBindings.emplace_back(camera.cameraDataDescFragment);
	allBindings.emplace_back(camera.cameraDataDescMesh);
	allBindings.emplace_back(camera.cameraDataDescAmplification);
	allBindings.emplace_back(Material::materialBufferBinding);
	allBindings.insert(allBindings.end(), MeshPool::bindingDescs.begin(), MeshPool::bindingDescs.end());
	allBindings.insert(allBindings.end(), Scene::bindingDescs.begin(), Scene::bindingDescs.end());
	for (const auto& textureBindings : Texture::textureBufferBindings)
		allBindings.emplace_back(textureBindings);

	auto set = device->CreateBindingSet(layout);
	set->WriteBindings(allBindings);

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
