#pragma once
#include <memory>

#include "Device.h"
#include "QueueFamily.h"
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class CommandBuffer;
	class Swapchain;
	class Fence;
	class Semaphore;
	class Queue
	{
	public:
		Queue(const vkn::Device& device, const QueueFamily& familyIndex, const uint32_t index);
		const uint32_t& familyIndex() const;
		std::shared_ptr<Fence> submit(CommandBuffer& cb, const bool signalSemaphore = true);
		void present(CommandBuffer& cb, const vkn::Swapchain& swapchain);
		void idle();
		const VkQueue queue() const;
		const uint32_t timestampValidBits() const;
	private:

		const vkn::Device& device_;
		VkQueue queue_{ VK_NULL_HANDLE };
		uint32_t familyIndex_;
		uint32_t timestampValidBits_{};
	};
}