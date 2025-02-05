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
	int instanceDataOffset;

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

	void LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadMultiObjectSphereScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadChessScene(std::shared_ptr<Device> device, const Camera& camera);
	
	void UploadInstanceDataToGPU(std::shared_ptr<Device> device);

public:

	// Keep in sync with InstanceData in common.hlsl
	struct InstanceData
	{
		glm::mat4x4 objectToWorld;
	};

	std::vector<ModelInstance> instances;
	std::wstring name;

	static std::shared_ptr<Resource> instanceDataBuffer;
	static std::shared_ptr<View> instanceDataView;

	static std::vector<BindingDesc> bindingDescs;
	static std::vector<BindKey> bindKeys;

	Scene() = default;
	~Scene() = default;

	static std::shared_ptr<Scene> LoadHardcodedScene(std::shared_ptr<Device> device, Camera& camera);
};