#include "../../headers/impl/vulkan/MeshMemoryLocation.h"
#include "../../headers/impl/vulkan/QueueFamily.h"

using namespace gee;
vkn::GeometryMemoryLocation::GeometryMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize) :
	memory{ std::move(mem) },	
	vertexBuffer{ std::move(vertexBuff) },
	indexBuffer{ std::move(indexBuff) },
	indicesCount{ indicesSize }
{
}