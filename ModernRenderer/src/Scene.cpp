#include "Scene.hpp"
#include "ModelImporter.hpp"
#include "MeshPool.hpp"

std::shared_ptr<Resource> Scene::instanceDataBuffer;
std::shared_ptr<View> Scene::instanceDataView;
std::vector<BindingDesc> Scene::bindingDescs;
std::vector<BindKey> Scene::bindKeys;

void Scene::LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleSphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);
}

void Scene::LoadMultiObjectSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"4 Sphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);
	ModelImporter importer2("assets/models/Cube.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer2.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(ModelInstance(importer.GetModel(), MatrixUtils::Translation(glm::vec3(-1, 0, -1))));
	instances.push_back(ModelInstance(importer.GetModel(), MatrixUtils::Translation(glm::vec3(1, 0, -1))));
	instances.push_back(ModelInstance(importer2.GetModel(), MatrixUtils::Translation(glm::vec3(-1, 0, 1))));
	instances.push_back(ModelInstance(importer2.GetModel(), MatrixUtils::Translation(glm::vec3(1, 0, 1))));
}

void Scene::LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleCube";

	ModelImporter importer("assets/models/MeshShaderTestPlane.fbx", aiProcessPreset_TargetRealtime_Fast);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);
}

void Scene::LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Sponza";

	ModelImporter importer("assets/models/Sponza/NewSponza_Main_glTF_003.gltf", 0);

	ModelInstance instance;
	instance.model = importer.GetModel();
	instance.transform = glm::mat4(0.0f);

	instances.push_back(instance);
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
	//scene->LoadSingleSphereScene(device, camera);
	scene->LoadMultiObjectSphereScene(device, camera);
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

void Scene::UploadInstanceDataToGPU(std::shared_ptr<Device> device)
{
	std::vector<InstanceData> instanceData;
	int index = 0;
	for (auto& instance : instances)
	{
		InstanceData data;
		data.objectToWorld = instance.transform;
		instanceData.push_back(data);
		instance.instanceDataOffset = index++;
	}

	instanceDataBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, sizeof(InstanceData) * instances.size());
	instanceDataBuffer->CommitMemory(MemoryType::kDefault);
	instanceDataBuffer->UpdateUploadBuffer(0, instanceData.data(), sizeof(InstanceData) * instances.size());
	instanceDataBuffer->SetName("InstanceDataBuffer");

	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kStructuredBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	viewDesc.buffer_size = sizeof(GPUMaterial) * instances.size();
	viewDesc.structure_stride = sizeof(InstanceData);
	instanceDataView = device->CreateView(instanceDataBuffer, viewDesc);

	auto instanceDataMeshBindKey = BindKey{ ShaderType::kMesh, ViewType::kStructuredBuffer, 2, 0 };
	auto instanceDataFragmentBindKey = BindKey{ ShaderType::kPixel, ViewType::kStructuredBuffer, 2, 0 };

	bindingDescs = {
		BindingDesc{ instanceDataMeshBindKey, instanceDataView },
		BindingDesc{ instanceDataFragmentBindKey, instanceDataView },
	};

	bindKeys = {
		instanceDataMeshBindKey,
		instanceDataFragmentBindKey,
	};
}
