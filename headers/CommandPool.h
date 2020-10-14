#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "Commandbuffer.h"
#include <vector>
namespace vkn
{
	class CommandPool
	{
	public:
		CommandPool(vkn::Device& device, const uint32_t familyIndex, const VkCommandPoolCreateFlagBits type);
		CommandPool(CommandPool&& other);
		~CommandPool();
		CommandBuffer getCommandBuffer(const VkCommandBufferLevel level);
	private:
		vkn::Device& device_;
		VkCommandPool pool_{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> cbs_;
	};
}