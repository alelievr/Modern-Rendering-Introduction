#include "Scene.hpp"
#include "ModelImporter.hpp"
#include "MeshPool.hpp"
#include <Utilities/Common.h>
#include "RenderUtils.hpp"
#include <CommandQueue/DXCommandQueue.h>

std::shared_ptr<Resource> Scene::instanceDataBuffer;
std::shared_ptr<View> Scene::instanceDataView;
std::shared_ptr<Resource> Scene::rtInstanceDataBuffer;
std::shared_ptr<View> Scene::rtInstanceDataView;
std::shared_ptr<Resource> Scene::visibleMeshletsBuffer0;
std::shared_ptr<View> Scene::visibleMeshletsView0;
std::shared_ptr<Resource> Scene::visibleMeshletsBuffer1;
std::shared_ptr<View> Scene::visibleMeshletsView1;
std::vector<BindingDesc> Scene::bindingDescs;
std::vector<BindKey> Scene::bindKeys;
BindKey Scene::accelerationStructureKey;
BindingDesc Scene::accelerationStructureBinding;

void Scene::LoadSingleSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"SingleSphere";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);
	instances.push_back(ModelInstance(importer.GetModel(), transpose(MatrixUtils::Translation(glm::vec3(0, 0, 0)))));
}

void Scene::LoadRoughnessTestScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Roughness Test";

	ModelImporter importer("assets/models/sphere.fbx", aiProcessPreset_TargetRealtime_Fast);
	auto model = importer.GetModel();
	auto mesh = model.parts[0].mesh;

	int spheresCount = 10;
	for (int x = 0; x < spheresCount; x++)
	{
		for (int z = 0; z < spheresCount; z++)
		{
			auto mat = Material::CreateMaterial();
			mat->roughness = (float)x / spheresCount;
			mat->metalness = (float)z / spheresCount;
			auto instance = ModelInstance(Model(mesh, mat), transpose(MatrixUtils::Translation(glm::vec3(x * 2.1, 0, z * 2.1))));
			instances.push_back(instance);
		}
	}
}

void Scene::LoadMultiObjectSphereScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Multi Objects";

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

void Scene::LoadTooMuchChessScene(std::shared_ptr<Device> device, const Camera& camera)
{
	name = L"Roughness Test";

	ModelImporter importer("assets/models/ABeautifulGame/glTF/ABeautifulGame.gltf", aiProcessPreset_TargetRealtime_Fast);
	auto model = importer.GetModel();
	auto mesh = model.parts[0].mesh;

	int chessCount = 10;
	for (int x = 0; x < chessCount; x++)
	{
		for (int z = 0; z < chessCount; z++)
		{
			auto instance = ModelInstance(model, transpose(MatrixUtils::Translation(glm::vec3(x * 0.71, 0, z * 0.71))));
			instances.push_back(instance);
		}
	}
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
	//scene->LoadRoughnessTestScene(device, camera);
	//scene->LoadMultiObjectSphereScene(device, camera);
	//scene->LoadStanfordBunnyScene(device, camera);
	//scene->LoadChessScene(device, camera);
	scene->LoadTooMuchChessScene(device, camera);
	//scene->LoadSponzaScene(device, camera);

	Texture::LoadAllMaterialTextures(device);
	Material::AllocateMaterialBuffers(device);
	scene->UploadInstancesToGPU(device);

	scene->sky.LoadHDRI(device, "assets/HDRIs/rogland_overcast_8k.hdr");
	//scene->sky.LoadHDRI(device, "assets/HDRIs/lenong_2_8k.hdr");
	//scene->sky.LoadHDRI(device, "assets/HDRIs/sunflowers_puresky_8k.hdr");
	scene->sky.Initialize(device , &camera);

	return scene;
}

void Scene::UploadInstancesToGPU(std::shared_ptr<Device> device)
{
	// Allocate and upload the mesh pool to the GPU
	MeshPool::AllocateMeshPoolBuffers(device);

	BuildRTAS(device);

	// Prepate and upload instance data
	std::vector<RTInstanceData> rtInstanceData;
	size_t maxMeshletsVisible = 0;
	int index = 0;
	for (auto& instance : instances)
	{
		for (auto& p : instance.model.parts)
		{
			InstanceData data;
			data.objectToWorld = instance.transform;
			data.meshletIndex = p.mesh->meshletOffset;
			data.materialIndex = p.material->materialIndex;
			data.meshletCount = p.mesh->meshletCount;
			data.obb = OBB(p.mesh->aabb, instance.transform);
			instanceData.push_back(data);

			RTInstanceData rtData;
			rtData.indexBufferOffset = p.mesh->raytracedPrimitiveIndex;
			rtData.materialIndex = p.material->materialIndex;
			rtInstanceData.push_back(rtData);

			instance.instanceDataOffset = index++;
			maxMeshletsVisible += p.mesh->meshletCount;
		}
	}

	size_t instanceDataSize = sizeof(InstanceData) * instanceData.size();
	instanceDataBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, instanceDataSize);
	instanceDataBuffer->CommitMemory(MemoryType::kUpload);
	instanceDataBuffer->UpdateUploadBuffer(0, instanceData.data(), instanceDataSize);
	instanceDataBuffer->SetName("Instance Data");

	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kStructuredBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	viewDesc.buffer_size = instanceDataSize;
	viewDesc.structure_stride = sizeof(InstanceData);
	instanceDataView = device->CreateView(instanceDataBuffer, viewDesc);

	size_t rtInstanceDataSize = sizeof(RTInstanceData) * rtInstanceData.size();
	rtInstanceDataBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, rtInstanceDataSize);
	rtInstanceDataBuffer->CommitMemory(MemoryType::kUpload);
	rtInstanceDataBuffer->UpdateUploadBuffer(0, rtInstanceData.data(), rtInstanceDataSize);
	rtInstanceDataBuffer->SetName("RT Instance Data");

	viewDesc.buffer_size = rtInstanceDataSize;
	viewDesc.structure_stride = sizeof(RTInstanceData);
	rtInstanceDataView = device->CreateView(rtInstanceDataBuffer, viewDesc);

	// Create 2 meshlet buffers for the culling results that have 2 passes
	size_t visibleMeshletsSize = sizeof(int) * 2 * maxMeshletsVisible;
	visibleMeshletsBuffer0 = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kUnorderedAccess, visibleMeshletsSize);
	visibleMeshletsBuffer0->CommitMemory(MemoryType::kDefault);
	visibleMeshletsBuffer0->SetName("Visible Meshlets 0");

	viewDesc.view_type = ViewType::kRWStructuredBuffer;
	viewDesc.buffer_size = visibleMeshletsSize;
	viewDesc.structure_stride = sizeof(int) * 2;
	visibleMeshletsView0 = device->CreateView(visibleMeshletsBuffer0, viewDesc);

	visibleMeshletsBuffer1 = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kUnorderedAccess, visibleMeshletsSize);
	visibleMeshletsBuffer1->CommitMemory(MemoryType::kDefault);
	visibleMeshletsBuffer1->SetName("Visible Meshlets 1");

	visibleMeshletsView1 = device->CreateView(visibleMeshletsBuffer1, viewDesc);

	auto instanceDataMeshBindKey = BindKey{ ShaderType::kUnknown, ViewType::kStructuredBuffer, 2, 0 };
	auto rtInstanceDataMeshBindKey = BindKey{ ShaderType::kUnknown, ViewType::kStructuredBuffer, 3, 0 };
	auto visibleMeshletsBindKey0 = BindKey{ ShaderType::kUnknown, ViewType::kRWStructuredBuffer, 0, 4 };
	auto visibleMeshletsBindKey1 = BindKey{ ShaderType::kUnknown, ViewType::kRWStructuredBuffer, 1, 4 };

	bindingDescs = {
		BindingDesc{ instanceDataMeshBindKey, instanceDataView },
		BindingDesc{ rtInstanceDataMeshBindKey, rtInstanceDataView },
		BindingDesc{ visibleMeshletsBindKey0, visibleMeshletsView0 },
		BindingDesc{ visibleMeshletsBindKey1, visibleMeshletsView1 },
	};

	bindKeys = {
		instanceDataMeshBindKey,
		rtInstanceDataMeshBindKey,
		visibleMeshletsBindKey0,
		visibleMeshletsBindKey1,
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
	
	size_t instanceCount = 0;
	for (const auto& instance : instances)
		for (auto& p : instance.model.parts)
			instanceCount++;

    auto tlasPrebuildInfo = device->GetTLASPrebuildInfo(instanceCount, BuildAccelerationStructureFlags::kNone);
	uint64_t tlasSize = Align(tlasPrebuildInfo.acceleration_structure_size, kAccelerationStructureAlignment);
	tlasBuffer = device->CreateBuffer(BindFlag::kAccelerationStructure, tlasSize);
	tlasBuffer->CommitMemory(MemoryType::kDefault);
	tlasBuffer->SetName("Top Level Acceleration Structures");

	scratchSize = std::max(scratchSize, tlasPrebuildInfo.build_scratch_data_size);
	scratch = device->CreateBuffer(BindFlag::kRayTracing, scratchSize);
	scratch->CommitMemory(MemoryType::kDefault);
	scratch->SetName("scratch");

	// Create BLAS for each mesh
	uint64_t offset = 0;
	for (auto mesh : MeshPool::meshes)
	{
		mesh.first->CreateBLAS(device, blasBuffer, offset, scratch);
		offset = Align(offset + mesh.first->blasCompactedSize, kAccelerationStructureAlignment);
	}

	// Create instances for the TLAS
	unsigned index = 0; // index into the rtInstanceData array.
    std::vector<RaytracingGeometryInstance> rtInstances;
    for (const auto& instance : instances)
	{
		for (auto& p : instance.model.parts)
		{
			RaytracingGeometryInstance& rt = rtInstances.emplace_back();
			rt.transform = glm::mat3x4(instance.transform);
			rt.flags = RaytracingInstanceFlags::kTriangleFrontCounterclockwise;
			rt.instance_offset = 0; // This instance offset is used to determine the hit index of the shader table, TODO: multiple shader support
			rt.instance_mask = 0xff;
			rt.instance_id = index++; // Pass the index of the first primitive of the mesh so that the hit shader can fetch vertex data
			rt.acceleration_structure_handle = p.mesh->blas->GetAccelerationStructureHandle();
		}
    }

	tlas = device->CreateAccelerationStructure(AccelerationStructureType::kTopLevel, tlasBuffer, 0);

	rtGeomInstanceDataBuffer = device->CreateBuffer(BindFlag::kRayTracing, rtInstances.size() * sizeof(RaytracingGeometryInstance));
	rtGeomInstanceDataBuffer->CommitMemory(MemoryType::kUpload);
	rtGeomInstanceDataBuffer->SetName("Instance Data");
	rtGeomInstanceDataBuffer->UpdateUploadBuffer(0, rtInstances.data(), rtInstances.size() * sizeof(RaytracingGeometryInstance));
    cmd->BuildTopLevelAS({}, tlas, scratch, 0, rtGeomInstanceDataBuffer, 0, rtInstances.size(), BuildAccelerationStructureFlags::kNone);
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