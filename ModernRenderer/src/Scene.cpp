#include "Scene.hpp"
#include "ModelImporter.hpp"
#include "MeshPool.hpp"
#include <Utilities/Common.h>
#include "RenderUtils.hpp"
#include <CommandQueue/DXCommandQueue.h>

std::shared_ptr<Resource> Scene::instanceDataBuffer;
std::shared_ptr<View> Scene::instanceDataView;
std::vector<BindingDesc> Scene::bindingDescs;
std::vector<BindKey> Scene::bindKeys;
BindKey Scene::accelerationStructureKey;
BindingDesc Scene::accelerationStructureBinding;

void Scene::LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleSphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);
	instances.push_back(ModelInstance(importer.GetModel()));
}

void Scene::LoadMultiObjectSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"4 Sphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);
	ModelImporter importer2("assets/models/Cube.fbx", aiProcessPreset_TargetRealtime_Fast);

	instances.push_back(ModelInstance(importer.GetModel(), transpose(MatrixUtils::Translation(glm::vec3(-1, 0, -1)))));
	instances.push_back(ModelInstance(importer.GetModel(), transpose(MatrixUtils::Translation(glm::vec3(1, 0, -1)))));
	instances.push_back(ModelInstance(importer2.GetModel(), transpose(MatrixUtils::Translation(glm::vec3(-1, 0, 1)))));
	instances.push_back(ModelInstance(importer2.GetModel(), transpose(MatrixUtils::Translation(glm::vec3(1, 0, 1)))));
}

void Scene::LoadSingleCubeScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleCube";

	ModelImporter importer("assets/models/Cube.fbx", aiProcessPreset_TargetRealtime_Fast);
	instances.push_back(ModelInstance(importer.GetModel()));
}

void Scene::LoadSponzaScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Sponza";

	ModelImporter importer("assets/models/Sponza/NewSponza_Main_glTF_003.gltf", 0);
	instances.push_back(ModelInstance(importer.GetModel()));
}

void Scene::LoadChessScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Chess";

	ModelImporter importer("assets/models/ABeautifulGame/glTF/ABeautifulGame.gltf", aiProcessPreset_TargetRealtime_Fast);
	instances.push_back(ModelInstance(importer.GetModel()));
}

void Scene::LoadStanfordBunnyScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Stanford Bunny";

	ModelImporter importer("assets/models/stanford-bunny.obj", aiProcessPreset_TargetRealtime_Fast);
	instances.push_back(ModelInstance(importer.GetModel()));
}

std::shared_ptr<Scene> Scene::LoadHardcodedScene(std::shared_ptr<Device> device, Camera& camera)
{
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	//scene->LoadSingleCubeScene(device, camera);
	//scene->LoadSingleSphereScene(device, camera);
	scene->LoadMultiObjectSphereScene(device, camera);
	//scene->LoadStanfordBunnyScene(device, camera);
	//scene->LoadChessScene(device, camera);

	Texture::LoadAllTextures(device);
	Material::AllocateMaterialBuffers(device);
	scene->UploadInstancesToGPU(device);

	scene->sky.LoadHDRI(device, "assets/HDRIs/rogland_overcast_8k.hdr");
	scene->sky.Initialize(device , &camera);

	return scene;
}

void Scene::UploadInstancesToGPU(std::shared_ptr<Device> device)
{
	// Allocate and upload the mesh pool to the GPU
	MeshPool::AllocateMeshPoolBuffers(device);

	BuildRTAS(device);

	// Prepate and upload instance data
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
	instanceDataBuffer->CommitMemory(MemoryType::kUpload);
	instanceDataBuffer->UpdateUploadBuffer(0, instanceData.data(), sizeof(InstanceData) * instances.size());
	instanceDataBuffer->SetName("InstanceDataBuffer");

	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kStructuredBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	viewDesc.buffer_size = sizeof(InstanceData) * instances.size();
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

void Scene::BuildRTAS(std::shared_ptr<Device> device)
{
	for (auto mesh : MeshPool::meshes)
		mesh.first->PrepareBLASData(device);

	auto cmd = device->CreateCommandList(CommandListType::kGraphics);
	cmd->SetName("TLAS Build Command List");
	auto uploadQueue = device->GetCommandQueue(CommandListType::kGraphics);

	uint64_t fenceValue = 0;
	std::shared_ptr<Fence> fence = device->CreateFence(fenceValue);

	uint64_t scratchSize = 0;
	uint64_t blasSize = 0;
	for (auto mesh : MeshPool::meshes)
	{
		blasSize += Align(mesh.first->blasPrebuildInfo.acceleration_structure_size, kAccelerationStructureAlignment);
		scratchSize = std::max(scratchSize, mesh.first->blasPrebuildInfo.acceleration_structure_size);
	}

    blasBuffer = device->CreateBuffer(BindFlag::kAccelerationStructure, blasSize);
    blasBuffer->CommitMemory(MemoryType::kDefault);
    blasBuffer->SetName("Bottom Level Acceleration Structures");
	
    auto tlasPrebuildInfo = device->GetTLASPrebuildInfo(MeshPool::meshes.size(), BuildAccelerationStructureFlags::kNone);
	uint64_t tlasSize = Align(tlasPrebuildInfo.acceleration_structure_size, kAccelerationStructureAlignment);
	tlasBuffer = device->CreateBuffer(BindFlag::kAccelerationStructure, tlasSize);
	tlasBuffer->CommitMemory(MemoryType::kDefault);
	tlasBuffer->SetName("Top Level Acceleration Structures");

	scratchSize = std::max(scratchSize, tlasPrebuildInfo.build_scratch_data_size);
	auto scratch = device->CreateBuffer(BindFlag::kRayTracing, scratchSize);
	scratch->CommitMemory(MemoryType::kDefault);
	scratch->SetName("scratch");

	// Create BLAS for each mesh
	uint64_t offset = 0;
	for (auto mesh : MeshPool::meshes)
	{
		mesh.first->CreateBLAS(device, blasBuffer, offset, scratch);
		offset = Align(offset + mesh.first->blas_compacted_size, kAccelerationStructureAlignment);
	}

	// Create instances for the TLAS
    std::vector<RaytracingGeometryInstance> rtInstances;
    for (const auto& instance : instances)
	{
		for (auto& p : instance.model.parts)
		{
			RaytracingGeometryInstance& rt = rtInstances.emplace_back();
			rt.transform = glm::mat3x4(instance.transform);
			rt.instance_offset = 0; // This instance offset is used to determine the hit index of the shader table, TODO: multiple shader support
			rt.instance_mask = 0xff;
			rt.acceleration_structure_handle = p.mesh->blas->GetAccelerationStructureHandle();
		}
    }

	tlas = device->CreateAccelerationStructure(AccelerationStructureType::kTopLevel, tlasBuffer, 0);

	rtInstanceDataBuffer = device->CreateBuffer(BindFlag::kRayTracing, rtInstances.size() * sizeof(RaytracingGeometryInstance));
	rtInstanceDataBuffer->CommitMemory(MemoryType::kUpload);
	rtInstanceDataBuffer->SetName("Instance Data");
	rtInstanceDataBuffer->UpdateUploadBuffer(0, rtInstances.data(), rtInstances.size() * sizeof(RaytracingGeometryInstance));
    cmd->BuildTopLevelAS({}, tlas, scratch, 0, rtInstanceDataBuffer, 0, rtInstances.size(),
        BuildAccelerationStructureFlags::kNone);
    cmd->UAVResourceBarrier(tlas);

    cmd->Close();

    uploadQueue->ExecuteCommandLists({ cmd });
    uploadQueue->Signal(fence, ++fenceValue);
	fence->Wait(fenceValue);
	uploadQueue->Wait(fence, fenceValue);

    ViewDesc tlasViewDesc = {};
    tlasViewDesc.view_type = ViewType::kAccelerationStructure;
	tlasView = device->CreateView(tlas, tlasViewDesc);

	accelerationStructureKey = BindKey{ ShaderType::kLibrary, ViewType::kAccelerationStructure, 1, 0 };
	accelerationStructureBinding = BindingDesc{ accelerationStructureKey, tlasView };
}