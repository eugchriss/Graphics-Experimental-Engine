#pragma once
#include "vulkanContext.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Queue.h"

namespace vkn
{
	struct MeshMemoryLocation
	{
		MeshMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize);
		MeshMemoryLocation(MeshMemoryLocation&&) = default;
		vkn::DeviceMemory memory;
		vkn::Buffer vertexBuffer;
		vkn::Buffer indexBuffer;
		const uint32_t indicesCount;
	};

	class MeshMemoryLocationFactory
	{
	public:
		MeshMemoryLocationFactory(Context& context);
		MeshMemoryLocation create(const gee::Mesh& mesh);
	private:
		Context& context_;
		std::unique_ptr<vkn::Queue> transferQueue_;
	};
}