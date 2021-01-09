#include "../headers/Queue.h"
#include "../headers/vulkan_utils.h"
#include <vector>

const uint32_t& vkn::Queue::familyIndex() const
{
	return familyIndex_;
}

void vkn::Queue::submit(const CommandBuffer& cb)
{
	auto cmdBuffer = cb.commandBuffer();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	vkn::error_check(vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE), "Unabled to command buffer");

}

void vkn::Queue::submit(const CommandBuffer& cb, vkn::Signal& submittedSignal, const bool persistent)
{
	auto cmdBuffer = cb.commandBuffer();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	if (persistent)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &submittedSignal.semaphore;
	}
	else
	{
		submitInfo.signalSemaphoreCount = 0;
	}
	vkn::error_check(vkQueueSubmit(queue_, 1, &submitInfo, submittedSignal.fence), "Unabled to command buffer");
}

void vkn::Queue::submit(const CommandBuffer& cb, vkn::Signal& submittedSignal, Signal& waitOn, const VkPipelineStageFlags waitingStage, const bool persistent)
{
	const auto& cmdBuffer = cb.commandBuffer();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitOn.semaphore;
	submitInfo.pWaitDstStageMask = &waitingStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	if (persistent)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &submittedSignal.semaphore;
	}
	else
	{
		submitInfo.signalSemaphoreCount = 0;
	}

	vkn::error_check(vkQueueSubmit(queue_, 1, &submitInfo, submittedSignal.fence), "Unabled to command buffer");

}

void vkn::Queue::present(const vkn::Swapchain& swapchain, Signal& waitOn)
{
	auto presentInfo = swapchain.imagePresentInfo(waitOn);
	vkn::error_check(vkQueuePresentKHR(queue_, &presentInfo), "Failed to present images");
}

void vkn::Queue::idle()
{
	vkQueueWaitIdle(queue_);
}

const VkQueue vkn::Queue::queue() const
{
	return queue_;
}

const uint32_t vkn::Queue::timestampValidBits() const
{
	return timestampValidBits_;
}

vkn::Queue::Queue(const vkn::Device& device, const vkn::QueueFamily& queueFamily, const uint32_t index) : device_{ device }, familyIndex_{ queueFamily.familyIndex() }
{
	vkGetDeviceQueue(device_.device, queueFamily.familyIndex(), index, &queue_);
	timestampValidBits_ = queueFamily.timestampValidBits();
}