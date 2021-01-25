#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include <string>
namespace vkn
{
	class CommandBuffer
	{
	public:
#ifndef NDEBUG
		void setDebugName(const std::string&);
#endif
		void begin(const VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		VkCommandBuffer commandBuffer() const;
		CommandBuffer(CommandBuffer&&) = default;
	private:
		friend class CommandPool;
		CommandBuffer(Context& context, const VkCommandBuffer cb);
		Context& context_;
		VkCommandBuffer cb_{ VK_NULL_HANDLE };
	};
}