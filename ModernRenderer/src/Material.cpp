#include "Material.hpp"

std::vector<std::shared_ptr<Material>> Material::instances = std::vector<std::shared_ptr<Material>>();
std::vector<GPUMaterial> Material::materialBuffer = std::vector<GPUMaterial>();
std::shared_ptr<Resource> Material::materialConstantBuffer = nullptr;
std::shared_ptr<View> Material::materialConstantBufferView = nullptr;

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
				if (parameter.name == "albedo")
				{
					gpuMaterial.albedoTextureIndex = parameter.textureValue->shaderResourceView->GetDescriptorId();
				}
			}
		}

		material->materialIndex = index++;
		materialBuffer.push_back(gpuMaterial);
	}

	int materialCount = materialBuffer.size();

	if (materialCount == 0)
		return;

	//materialConstantBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, sizeof(GPUMaterial) * materialCount);
	//materialConstantBuffer->CommitMemory(MemoryType::kUpload);
	//materialConstantBuffer->UpdateUploadBuffer(0, &materialBuffer, sizeof(GPUMaterial) * materialCount);
	//ViewDesc viewDesc = {};
	//viewDesc.view_type = ViewType::kBuffer;
	//viewDesc.dimension = ViewDimension::kBuffer;
	//materialConstantBufferView = device->CreateView(materialConstantBuffer, viewDesc);
}
