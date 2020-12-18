#pragma once
#include "DeviceMemory.h"
#include "Buffer.h"
#include "Mesh.h"

namespace vkn
{
	struct MeshMemoryLocation
	{
		MeshMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize);
		vkn::DeviceMemory memory;
		vkn::Buffer vertexBuffer;
		vkn::Buffer indexBuffer;
		const uint32_t indicesCount;
	};
	class MeshMemoryLocationFactory
	{
	public:
		MeshMemoryLocationFactory(vkn::Gpu& gpu, vkn::Device& device);
		MeshMemoryLocation create(const gee::Mesh& mesh);
	private:
		vkn::Gpu& gpu_;
		vkn::Device& device_;
	};
}
