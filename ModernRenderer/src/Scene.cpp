#include "Scene.hpp"
#include "ModelImporter.hpp"
#include "MeshPool.hpp"

void Scene::LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleSphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);

	UploadInstancesToGPU(device);
}

void Scene::LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleCube";

	ModelImporter importer("assets/models/MeshShaderTestPlane.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);

	UploadInstancesToGPU(device);
}

void Scene::LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Sponza";

	ModelImporter importer("assets/models/Sponza/NewSponza_Main_glTF_003.gltf", 0);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);

	UploadInstancesToGPU(device);
}

void Scene::LoadChessScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Chess";

	ModelImporter importer("assets/models/ABeautifulGame/glTF/ABeautifulGame.gltf", 0);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);
}

std::shared_ptr<Scene> Scene::LoadHardcodedScene(std::shared_ptr<Device> device, Camera& camera)
{
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	//scene->LoadSingleCubeScene(device, camera);
	scene->LoadSingleSphereScene(device, camera);
	//scene->LoadChessScene(device, camera);

	scene->UploadInstancesToGPU(device);
	Texture::LoadAllTextures(device);
	Material::AllocateMaterialBuffers(device);

	return scene;
}

void Scene::UploadInstancesToGPU(std::shared_ptr<Device> device)
{
	for (auto& instance : instances)
	{
		for (auto& p : instance.model.parts)
			p.mesh.PrepareMeshletData(device);
	}

	// Allocate and upload the mesh pool to the GPU
	MeshPool::AllocateMeshPoolBuffers(device);
}
