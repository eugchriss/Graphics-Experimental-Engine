#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Commandbuffer.h"
#include <map>
#include <vector>
#include <queue>
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