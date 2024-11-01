#include "Scene.hpp"
#include "ModelImporter.hpp"

void Scene::LoadHardcodedScene(std::shared_ptr<Device> device, Camera camera)
{
	name = L"Hardcoded Scene 00";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);

	UploadInstancesToGPU(device);
}

void Scene::UploadInstancesToGPU(std::shared_ptr<Device> device)
{
	for (auto& instance : instances)
	{
		for (auto& p : instance.model.parts)
		{
			p.mesh.UploadMeshData(device);
		}	
	}
}
