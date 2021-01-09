#include "..\headers\meshMemoryLocation.h"
#include "../headers/QueueFamily.h"

vkn::MeshMemoryLocation::MeshMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize) :
	memory{ std::move(mem) },	
	vertexBuffer{ std::move(vertexBuff) },
	indexBuffer{ std::move(indexBuff) },
	indicesCount{ indicesSize }
{
}
vkn::MeshMemoryLocationFactory::MeshMemoryLocationFactory(vkn::Gpu& gpu, vkn::Device& device) : gpu_{ gpu }, device_{ device }
{
	vkn::QueueFamily queueFamily{ gpu_, VK_QUEUE_TRANSFER_BIT, 1 };
	transferQueue_ = queueFamily.getQueue(device_);
}

vkn::MeshMemoryLocation vkn::MeshMemoryLocationFactory::create(const gee::Mesh& mesh)
{
	const auto& vertices = mesh.vertices();
	auto verticesSize = std::size(vertices) * sizeof(gee::Vertex);
	vkn::Buffer vertexBuffer{ device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, verticesSize };

	const auto& indices = mesh.indices();
	auto indicesSize = std::size(indices) * sizeof(uint32_t);
	vkn::Buffer indexBuffer{ device_, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indicesSize };

	vkn::DeviceMemory temp{ gpu_, device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
	vertexBuffer.bind(temp);
	indexBuffer.bind(temp);

	vertexBuffer.add(vertices);
	indexBuffer.add(indices);
	
	vkn::DeviceMemory memory{ gpu_, device_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
	vertexBuffer.moveTo(*transferQueue_, memory);
	indexBuffer.moveTo(*transferQueue_, memory);
	return vkn::MeshMemoryLocation{ std::move(memory), std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(std::size(indices)) };
}