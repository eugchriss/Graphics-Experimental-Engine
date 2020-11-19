#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
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
	private:
		friend class CommandPool;
		CommandBuffer(vkn::Device& device, const VkCommandBuffer cb);
		vkn::Device& device_;
		VkCommandBuffer cb_{ VK_NULL_HANDLE };
	};
}