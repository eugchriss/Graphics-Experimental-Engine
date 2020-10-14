#pragma once
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class CommandBuffer
	{
	public:
		void begin(const VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		VkCommandBuffer commandBuffer() const;
	private:
		friend class CommandPool;
		CommandBuffer(const VkCommandBuffer cb);

		VkCommandBuffer cb_{ VK_NULL_HANDLE };
	};
}