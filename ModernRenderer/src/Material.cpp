#include "Material.hpp"


Material::Material()
{
	instances.push_back(this);
}

Material::~Material()
{
	instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
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

	int materialCount = instances.size();
	materialConstantBuffer = device->CreateBuffer(BindFlag::kShaderResource | BindFlag::kCopyDest, sizeof(GPUMaterial) * materialCount);
	materialConstantBuffer->CommitMemory(MemoryType::kUpload);
	materialConstantBuffer->UpdateUploadBuffer(0, &materialBuffer, sizeof(GPUMaterial) * materialCount);
	ViewDesc viewDesc = {};
	viewDesc.view_type = ViewType::kBuffer;
	viewDesc.dimension = ViewDimension::kBuffer;
	materialConstantBufferView = device->CreateView(materialConstantBuffer, viewDesc);
}
