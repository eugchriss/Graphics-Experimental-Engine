#include "../headers/Queue.h"
#include "../headers/vulkan_utils.h"
#include "../headers/Commandbuffer.h" 
#include "../headers/Swapchain.h"
#include "../headers/Signal.h"
#include <vector>

const uint32_t& vkn::Queue::familyIndex() const
{
	return familyIndex_;
}

std::shared_ptr<vkn::Fence> vkn::Queue::submit(CommandBuffer& cb)
{
	auto cmdBuffer = cb.commandBuffer();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	auto semaphore = cb.completedSemaphore();
	submitInfo.pSignalSemaphores = &semaphore;

	auto fence = std::make_shared<Fence>(cb.context(), false);
	vkn::error_check(vkQueueSubmit(queue_, 1, &submitInfo, (*fence)()), "Unabled to command buffer");
	cb.attachFence(fence);
	return fence;
}

void vkn::Queue::present(vkn::CommandBuffer& cb, const vkn::Swapchain& swapchain)
{
	auto presentInfo = swapchain.imagePresentInfo(cb.completedSemaphore);
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