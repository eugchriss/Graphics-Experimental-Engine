#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"

namespace vkn
{
	class Signal
	{
	public:
		Signal(Context& context, bool signaled = false);
		Signal(const Signal&) = delete;
		Signal(Signal&& other);
		~Signal();
		Signal& operator=(Signal&&) = delete;
		Signal& operator=(const Signal&) = delete;
		void reset();
		VkSemaphore semaphore{ VK_NULL_HANDLE };
		VkFence fence{ VK_NULL_HANDLE };
		bool signaled();
		void waitForSignal(const uint64_t timeout = UINT64_MAX) const;
	private:
		Context& context_;

#ifndef NDEBUG
		bool signaled_;
#endif
	};
}