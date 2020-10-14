#include "../headers/MemoryLocation.h"

vkn::MemoryLocation::MemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize) :
	memory{ std::move(mem) }, 
	vertexBuffer{ std::move(vertexBuff) },
	indexBuffer{std::move(indexBuff)},
	indicesCount{indicesSize}
{
}