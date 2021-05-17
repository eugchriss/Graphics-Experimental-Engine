#pragma once
#include <functional>
#include <memory>
#include <string>

#include "Signal.h"
#include "vulkanContext.h"
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class CommandBuffer
	{
	public:
#ifndef NDEBUG
		void setDebugName(const std::string&);
#endif
		CommandBuffer(Context& context, const VkCommandBuffer cb);
		CommandBuffer(CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) = default;
		void attachFence(std::shared_ptr<Fence>& fence);
		void begin(const VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		VkCommandBuffer commandBuffer() const;
		Semaphore completedSemaphore;
		const bool isPending() const;
		Context& context() const;
	private:
		friend class CommandPool;
		Context& context_;
		VkCommandBuffer cb_{ VK_NULL_HANDLE };
		std::shared_ptr<Fence> isPendingFence_;
	};

	MAKE_REFERENCE(CommandBuffer);
}