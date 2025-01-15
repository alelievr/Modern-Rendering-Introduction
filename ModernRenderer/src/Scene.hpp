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

	// Disable copies of scene
	Scene(const Scene&);
	Scene& operator=(const Scene&);

public:
	std::vector<ModelInstance> instances;
	std::wstring name;

	Scene() = default;
	~Scene() = default;

	void LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadChessScene(std::shared_ptr<Device> device, const Camera& camera);

	static std::shared_ptr<Scene> LoadHardcodedScene(std::shared_ptr<Device> device, Camera& camera);
};