#include "..\headers\MeshMemoryLocation.h"
#include "../headers/QueueFamily.h"

vkn::GeometryMemoryLocation::GeometryMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize) :
	memory{ std::move(mem) },	
	vertexBuffer{ std::move(vertexBuff) },
	indexBuffer{ std::move(indexBuff) },
	indicesCount{ indicesSize }
{
}
vkn::GeometryMemoryLocationFactory::GeometryMemoryLocationFactory(vkn::Context& context) : context_{context}
{
}

vkn::GeometryMemoryLocation vkn::GeometryMemoryLocationFactory::create(const gee::Geometry& geometry)
{
	const auto& vertices = geometry.vertices;
	auto verticesSize = std::size(vertices) * sizeof(gee::Vertex);
	vkn::Buffer vertexBuffer{ context_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, verticesSize };

	const auto& indices = geometry.indices;
	auto indicesSize = std::size(indices) * sizeof(uint32_t);
	vkn::Buffer indexBuffer{ context_, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indicesSize };

	vkn::DeviceMemory temp{ *context_.gpu, *context_.device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
	vertexBuffer.bind(temp);
	indexBuffer.bind(temp);

	vertexBuffer.add(vertices);
	indexBuffer.add(indices);
	
	vkn::DeviceMemory memory{ *context_.gpu, *context_.device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
	vertexBuffer.moveTo(*context_.transferQueue, memory);
	indexBuffer.moveTo(*context_.transferQueue, memory);
	return vkn::GeometryMemoryLocation{ std::move(memory), std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(std::size(indices)) };
}