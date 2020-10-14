#pragma once
#include "DeviceMemory.h"
#include "Buffer.h"

namespace vkn
{
	struct MemoryLocation
	{
		MemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize);
		vkn::DeviceMemory memory;
		vkn::Buffer vertexBuffer;
		vkn::Buffer indexBuffer;
		const uint32_t indicesCount;
	};
}
