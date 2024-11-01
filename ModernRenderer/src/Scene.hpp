#pragma once

#include <vector>
#include <string>

#include "Camera.hpp"
#include "Model.hpp"

class ModelInstance
{
	public:
	Model model;
	glm::mat4 transform;

	ModelInstance() = default;
	ModelInstance(Model model, glm::mat4 transform) : model(model), transform(transform) {}
	~ModelInstance() = default;
};

class Scene
{
private:
	void UploadInstancesToGPU(std::shared_ptr<Device> device);

public:
	std::vector<ModelInstance> instances;
	std::wstring name;

	Scene() = default;
	~Scene() = default;

	void LoadHardcodedScene(std::shared_ptr<Device> device, Camera camera);
};