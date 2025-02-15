#pragma once

#include <vector>
#include <string>

#include "Camera.hpp"
#include "Model.hpp"
#include "Sky.hpp"

class ModelInstance
{
	public:
	Model model;
	glm::mat4 transform = glm::mat4(1.0f);
	int instanceDataOffset = 0;

	ModelInstance() = default;
	ModelInstance(Model model, glm::mat4 transform = glm::mat4(1.0f)) : model(model), transform(transform) {}
	~ModelInstance() = default;
};

class Scene
{
private:
	// Disable copies of scene
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	void LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadMultiObjectSphereScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadChessScene(std::shared_ptr<Device> device, const Camera& camera);
	void LoadStanfordBunnyScene(std::shared_ptr<Device> device, const Camera& camera);
	
	void UploadInstancesToGPU(std::shared_ptr<Device> device);

	void BuildRTAS(std::shared_ptr<Device> device);

public:

	Scene() = default;
	~Scene() = default;

	static std::shared_ptr<Resource> instanceDataBuffer;
	static std::shared_ptr<View> instanceDataView;

	static std::vector<BindingDesc> bindingDescs;
	static std::vector<BindKey> bindKeys;

	static BindKey accelerationStructureKey;
	static BindingDesc accelerationStructureBinding;

	// Keep in sync with InstanceData in common.hlsl
	struct InstanceData
	{
		glm::mat4x4 objectToWorld;
	};

	std::vector<ModelInstance> instances;
	std::wstring name;
	std::shared_ptr<View> tlasView;
	std::shared_ptr<Resource> tlas;
	std::shared_ptr<Resource> tlasBuffer;
	std::shared_ptr<Resource> blasBuffer;
	std::shared_ptr<Resource> rtInstanceDataBuffer;

	Sky sky;

	static std::shared_ptr<Scene> LoadHardcodedScene(std::shared_ptr<Device> device, Camera& camera);
};