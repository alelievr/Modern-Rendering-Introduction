#include "Mesh.hpp"

template<typename T>
std::shared_ptr<Resource> AllocateVertexBufer(std::shared_ptr<Device> device, const std::vector<T>& data)
{
	auto buffer = device->CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(T) * data.size());
	buffer->CommitMemory(MemoryType::kUpload);
	buffer->UpdateUploadBuffer(0, data.data(), sizeof(data.front()) * data.size());
	return buffer;
}

void Mesh::UploadMeshData(std::shared_ptr<Device> device)
{
	// Allocate memory for the vertex buffer
	vertexPositionBuffer = AllocateVertexBufer<glm::vec3>(device, positions);
	vertexNormalBuffer = AllocateVertexBufer<glm::vec3>(device, normals);
	vertexTexcoordBuffer = AllocateVertexBufer<glm::vec2>(device, texcoords);
	vertexTangentBuffer = AllocateVertexBufer<glm::vec3>(device, tangents);

	// Allocate memory for the index buffer
	indexBuffer = device->CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * indices.size());
	indexBuffer->CommitMemory(MemoryType::kUpload);
	indexBuffer->UpdateUploadBuffer(0, indices.data(), sizeof(indices.front()) * indices.size());
}

void Mesh::BindBuffers(std::shared_ptr<CommandList> commandList)
{
	commandList->IASetVertexBuffer(0, vertexPositionBuffer);
	commandList->IASetVertexBuffer(1, vertexNormalBuffer);
	commandList->IASetVertexBuffer(2, vertexTexcoordBuffer);
	commandList->IASetVertexBuffer(3, vertexTangentBuffer);
	commandList->IASetIndexBuffer(indexBuffer, gli::FORMAT_R32_UINT_PACK32);
}