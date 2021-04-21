#pragma once
#include "vulkanContext.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Queue.h"

namespace vkn
{
	struct GeometryMemoryLocation
	{
		GeometryMemoryLocation(vkn::DeviceMemory&& mem, vkn::Buffer&& vertexBuff, vkn::Buffer&& indexBuff, const uint32_t indicesSize);
		GeometryMemoryLocation(GeometryMemoryLocation&&) = default;
		vkn::DeviceMemory memory;
		vkn::Buffer vertexBuffer;
		vkn::Buffer indexBuffer;
		const uint32_t indicesCount;
	};
	MAKE_REFERENCE(GeometryMemoryLocation);

	class GeometryMemoryLocationFactory
	{
	public:
		GeometryMemoryLocationFactory(Context& context);
		GeometryMemoryLocation create(const gee::Geometry& geometry);
	private:
		Context& context_;
		std::unique_ptr<vkn::Queue> transferQueue_;
	};
}