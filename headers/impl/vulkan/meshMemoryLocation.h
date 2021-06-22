#pragma once
#include "../../headers/geometry.h"
#include "../../headers/utility.h"

#include "Buffer.h"
#include "DeviceMemory.h"
#include "Queue.h"
#include "vulkanContext.h"

namespace gee
{
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
	}
	template<>
	struct ID<const vkn::GeometryMemoryLocationRef>
	{
		using Type = size_t;
		static Type get(const vkn::GeometryMemoryLocationRef& geom)
		{
			return Type{};
		}
	};

	template<class T> struct ResourceLoader;

	template<>
	struct ResourceLoader<vkn::GeometryMemoryLocation>
	{
		static vkn::GeometryMemoryLocation load(vkn::Context& context, GeometryConstRef& geometry)
		{
			const auto& vertices = geometry.get().vertices;
			auto verticesSize = std::size(vertices) * sizeof(gee::Vertex);
			vkn::Buffer vertexBuffer{ context, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, verticesSize };

			const auto& indices = geometry.get().indices;
			auto indicesSize = std::size(indices) * sizeof(uint32_t);
			vkn::Buffer indexBuffer{ context, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indicesSize };

			vkn::DeviceMemory temp{ *context.gpu, *context.device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
			vertexBuffer.bind(temp);
			indexBuffer.bind(temp);

			vertexBuffer.add(vertices);
			indexBuffer.add(indices);

			vkn::DeviceMemory memory{ *context.gpu, *context.device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
			vertexBuffer.moveTo(*context.transferQueue, memory);
			indexBuffer.moveTo(*context.transferQueue, memory);
			return vkn::GeometryMemoryLocation{ std::move(memory), std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(std::size(indices)) };
		}
	};
}