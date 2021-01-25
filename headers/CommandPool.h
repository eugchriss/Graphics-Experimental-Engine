#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Commandbuffer.h"
#include <vector>
namespace vkn
{
	class CommandPool
	{
	public:
		CommandPool(Context& _context, const VkCommandPoolCreateFlagBits type);
		CommandPool(CommandPool&& other);
		~CommandPool();
		CommandBuffer getCommandBuffer(const VkCommandBufferLevel level);
	private:
		Context& context_;
		VkCommandPool pool_{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> cbs_;
	};
}