#pragma once
#include <map>
#include <vector>
#include <queue>

#include "Commandbuffer.h"
#include "vulkanContext.h"
#include "vulkan/vulkan.hpp"
namespace vkn
{
	class CommandPool
	{
	public:
		CommandPool(Context& _context, const VkCommandPoolCreateFlagBits type, const uint32_t minCommandBufferCount = 10);
		CommandPool(CommandPool&& other);
		~CommandPool();
		CommandBuffer& getCommandBuffer(const VkCommandBufferLevel level);
	private:
		Context& context_;
		VkCommandPool pool_{ VK_NULL_HANDLE };
		struct CommandBufferLevel
		{
			std::vector<CommandBuffer> commandBuffers;
			std::queue<CommandBufferRef> availableCommandBuffers;
			std::vector<CommandBufferRef> pendingCommandBuffers;
		};
		std::map<VkCommandBufferLevel, CommandBufferLevel> commandBuffers_;
		void allocateCommandBuffer(const VkCommandBufferLevel level, const uint32_t count);
		void sortCompletedCommandBuffers();
	};
}