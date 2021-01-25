#pragma once
#include "vulkan/vulkan.hpp"
#include "QueueFamily.h"
#include "Device.h"

namespace vkn
{
	class CommandBuffer;
	class Swapchain;
	class Signal;
	class Queue
	{
	public:
		Queue(const vkn::Device& device, const QueueFamily& familyIndex, const uint32_t index);
		const uint32_t& familyIndex() const;
		void submit(const CommandBuffer& cb);
		void submit(const CommandBuffer& cb, vkn::Signal& submittedSignal, const bool persistent = false);
		void submit(const CommandBuffer& cb, vkn::Signal& submittedSignal, Signal& waitOn, const VkPipelineStageFlags waitingStage, const bool persistent = false);
		void present(const vkn::Swapchain& swapchain, Signal& waitOn);
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