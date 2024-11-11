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
	instances.push_back(shared);
	return shared;
}

void Material::AddTextureParameter(std::shared_ptr<Texture> texture)
{
	parameters.push_back({ texture->path, MaterialParameterType::Texture, texture });
}

void Material::AllocateMaterialBuffers(std::shared_ptr<Device> device)
{
	int index = 0;
	for (auto material : instances)
	{
		GPUMaterial gpuMaterial;
		gpuMaterial.albedoTextureIndex = -1;

		for (auto& parameter : material->parameters)
		{
			if (parameter.type == MaterialParameterType::Texture)
			{
				if (parameter.textureValue->type == PBRTextureType::Albedo)
				{
					gpuMaterial.albedoTextureIndex = parameter.textureValue->shaderResourceView->GetDescriptorId();
					printf("Albedo texture index: %d\n", gpuMaterial.albedoTextureIndex);
				}
			}
		}

		material->materialIndex = index++;
		materialBuffer.push_back(gpuMaterial);
	}

	int materialCount = materialBuffer.size();

	if (materialCount == 0)
		return;

	materialConstantBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, sizeof(GPUMaterial) * materialCount);
	materialConstantBuffer->CommitMemory(MemoryType::kUpload);
	materialConstantBuffer->SetName("MaterialDataBuffer");
	materialConstantBuffer->UpdateUploadBuffer(0, &materialBuffer, sizeof(GPUMaterial) * materialCount);
	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kStructuredBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	viewDesc.buffer_size = materialCount;
	viewDesc.structure_stride = sizeof(GPUMaterial);
	materialConstantBufferView = device->CreateView(materialConstantBuffer, viewDesc);

	materialBufferBindKey = { ShaderType::kPixel, ViewType::kStructuredBuffer, 0, 2, 1, UINT32_MAX };
	materialBufferBinding = { materialBufferBindKey, materialConstantBufferView };
}

bool Material::Compare(const std::shared_ptr<Material>& a, const std::shared_ptr<Material>& b)
{
	if (a->parameters.size() != b->parameters.size())
		return false;

	std::sort(a->parameters.begin(), a->parameters.end(), [](const MaterialParameter& a, const MaterialParameter& b) { return a.name < b.name; });
	std::sort(b->parameters.begin(), b->parameters.end(), [](const MaterialParameter& a, const MaterialParameter& b) { return a.name < b.name; });

	// Check all parameters
	for (int i = 0; i < a->parameters.size(); ++i)
	{
		if (a->parameters[i].name != b->parameters[i].name)
			return false;
		if (a->parameters[i].type != b->parameters[i].type)
			return false;
		if (a->parameters[i].textureValue != b->parameters[i].textureValue)
			return false;
		if (a->parameters[i].float4Value != b->parameters[i].float4Value)
			return false;
		if (a->parameters[i].floatValue != b->parameters[i].floatValue)
			return false;
		if (a->parameters[i].intValue != b->parameters[i].intValue)
			return false;
	}

	return true;
}
