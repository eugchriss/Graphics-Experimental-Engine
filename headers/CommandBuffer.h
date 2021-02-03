#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Signal.h"
#include <string>
#include <functional>
namespace vkn
{
	class CommandBuffer
	{
	public:
#ifndef NDEBUG
		void setDebugName(const std::string&);
#endif
		CommandBuffer(CommandBuffer&&) = default;
		CommandBuffer& operator=(CommandBuffer&&) = default;
		void begin(const VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void end();
		VkCommandBuffer commandBuffer() const;
		bool isComplete();
		Signal& completeSignal();
		CommandBuffer(Context& context, const VkCommandBuffer cb);
	private:
		friend class CommandPool;
		std::reference_wrapper<Context> context_;
		VkCommandBuffer cb_{ VK_NULL_HANDLE };
		Signal complete_;
	};
}