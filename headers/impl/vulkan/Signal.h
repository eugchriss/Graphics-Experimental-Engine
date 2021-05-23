#pragma once
#include <functional>

#include "vulkanContext.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		class Fence
		{
		public:
			Fence(Context& context, const bool signaled);
			Fence(Fence&&);
			~Fence();
			VkFence& operator()();
			void wait(const uint64_t timeout = UINT64_MAX) const;
			bool signaled() const;
			void reset();
		private:
			Context& context_;
			VkFence fence_{ VK_NULL_HANDLE };
		};

		class Semaphore
		{
		public:
			Semaphore(Context& context);
			Semaphore(Semaphore&& other);
			~Semaphore();

			VkSemaphore& operator()();
		private:
			Context& context_;
			VkSemaphore semaphore_{ VK_NULL_HANDLE };
		};
	}
}