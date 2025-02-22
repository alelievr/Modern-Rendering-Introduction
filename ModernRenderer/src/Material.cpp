#include "Material.hpp"
#include <algorithm>

std::vector<std::shared_ptr<Material>> Material::instances = std::vector<std::shared_ptr<Material>>();
std::vector<GPUMaterial> Material::materialBuffer = std::vector<GPUMaterial>();
std::shared_ptr<Resource> Material::materialConstantBuffer = nullptr;
std::shared_ptr<View> Material::materialConstantBufferView = nullptr;
BindingDesc Material::materialBufferBinding;
BindKey Material::materialBufferBindKey;

Material::~Material()
{
}

std::shared_ptr<Material> Material::CreateMaterial()
{
	auto shared = std::make_shared<Material>();
	return shared;
}

void Material::AddTextureParameter(std::shared_ptr<Texture> texture)
{
	switch (texture->type)
	{
		case PBRTextureType::BaseColor:
			baseColorTexture = texture;
			break;
		case PBRTextureType::Normal:
			normalTexture = texture;
			break;
		case PBRTextureType::Metalness:
			metalnessTexture = texture;
			break;
		case PBRTextureType::Roughness:
			roughnessTexture = texture;
			break;
		case PBRTextureType::SpecularColor:
			specularColorTexture = texture;
			break;
		case PBRTextureType::AmbientOcclusion:
			ambientOcclusion = texture;
			break;
	}
}

static int GetTextureBindlessIndex(std::shared_ptr<Texture> texture)
{
	if (texture == nullptr)
		return -1;

	return texture->shaderResourceView->GetDescriptorId();
}

void Material::AllocateMaterialBuffers(std::shared_ptr<Device> device)
{
	int index = 0;
	for (auto material : instances)
	{
		GPUMaterial gpuMaterial;
		
		gpuMaterial.baseColor = material->baseColor;
		gpuMaterial.baseColorTextureIndex = GetTextureBindlessIndex(material->baseColorTexture);
		gpuMaterial.metalness = material->metalness;
		gpuMaterial.metalnessTextureIndex = GetTextureBindlessIndex(material->metalnessTexture);
		gpuMaterial.diffuseRoughness = material->roughness;
		gpuMaterial.diffuseRoughnessTextureIndex = GetTextureBindlessIndex(material->roughnessTexture);
		gpuMaterial.normalTextureIndex = GetTextureBindlessIndex(material->normalTexture);
		gpuMaterial.ambientOcclusionTextureIndex = GetTextureBindlessIndex(material->ambientOcclusion);

		material->materialIndex = index++;
		materialBuffer.push_back(gpuMaterial);
	}

	int materialCount = materialBuffer.size();

	if (materialCount == 0)
		return;

	materialConstantBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, sizeof(GPUMaterial) * materialCount);
	materialConstantBuffer->CommitMemory(MemoryType::kDefault);
	materialConstantBuffer->SetName("MaterialDataBuffer");

	auto uploadBuffer = device->CreateBuffer(BindFlag::kCopySource, sizeof(GPUMaterial) * materialCount);
	uploadBuffer->CommitMemory(MemoryType::kUpload);
	uploadBuffer->SetName("MaterialDataUploadBuffer");
	uploadBuffer->UpdateUploadBuffer(0, materialBuffer.data(), sizeof(GPUMaterial) * materialCount);
	auto cmd = device->CreateCommandList(CommandListType::kCopy);
	BufferCopyRegion region = {};
	region.num_bytes = sizeof(GPUMaterial) * materialCount;
	cmd->ResourceBarrier({ { materialConstantBuffer, ResourceState::kCommon, ResourceState::kCopyDest } });
	cmd->CopyBuffer(uploadBuffer, materialConstantBuffer, { region });
	cmd->Close();
	std::shared_ptr<CommandQueue> queue = device->GetCommandQueue(CommandListType::kCopy);
	queue->ExecuteCommandLists({ cmd });
	std::shared_ptr<Fence> fence = device->CreateFence(0);
	queue->Signal(fence, 1);
	queue->Wait(fence, 1);
	fence->Wait(1);

	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kStructuredBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	viewDesc.buffer_size = sizeof(GPUMaterial) * materialCount;
	viewDesc.structure_stride = sizeof(GPUMaterial);
	materialConstantBufferView = device->CreateView(materialConstantBuffer, viewDesc);

	materialBufferBindKey = { ShaderType::kPixel, ViewType::kStructuredBuffer, 0, 2, 1, UINT32_MAX };
	materialBufferBinding = { materialBufferBindKey, materialConstantBufferView };
}

bool Material::Compare(const std::shared_ptr<Material>& a, const std::shared_ptr<Material>& b)
{
	if (!a || !b)
		return false;

	if (a->name != b->name) return false;

	if (a->specularWorkflow != b->specularWorkflow) return false;
	if (a->baseColor != b->baseColor) return false;
	if (a->metalness != b->metalness) return false;
	if (a->roughness != b->roughness) return false;
	if (a->specularColor != b->specularColor) return false;
	if (a->materialIndex != b->materialIndex) return false;

	// Textures are de-duplicated so it should work
	if (a->baseColorTexture != b->baseColorTexture) return false;
	if (a->metalnessTexture != b->metalnessTexture) return false;
	if (a->roughnessTexture != b->roughnessTexture) return false;
	if (a->specularColorTexture != b->specularColorTexture) return false;
	if (a->normalTexture != b->normalTexture) return false;
	if (a->ambientOcclusion != b->ambientOcclusion) return false;

	return true;
}
