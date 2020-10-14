#include "../headers/signal.h"
#include "../headers/vulkan_utils.h"

vkn::Signal::Signal(const vkn::Device& device, bool signaled) : device_{ device }
{
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	if (signaled)
	{
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	}
	else
	{
		fenceCI.flags = 0;
	}
	vkn::error_check(vkCreateFence(device_.device, &fenceCI, nullptr, &fence), "Unable to create the rendering finished fence");

	VkSemaphoreCreateInfo semaphoreCI{};
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;
	
	vkn::error_check(vkCreateSemaphore(device_.device, &semaphoreCI, nullptr, &semaphore), "Unable to create the rendering-finshed semaphore");
#ifndef NDEBUG
	signaled_ = signaled;
#endif
}

vkn::Signal::Signal(Signal&& other): device_{other.device_}
{
	fence = other.fence;
	semaphore = other.semaphore;

	other.fence = VK_NULL_HANDLE;
	other.semaphore = VK_NULL_HANDLE;
}

vkn::Signal::~Signal()
{
	if (semaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(device_.device, semaphore, nullptr);
	}
	if (fence != VK_NULL_HANDLE)
	{
		vkWaitForFences(device_.device, 1, &fence, VK_TRUE, 500000000);
		vkDestroyFence(device_.device, fence, nullptr);
	}
}

void vkn::Signal::reset()
{
#ifndef NDEBUG
	signaled_ = false;
#endif
	vkn::error_check(vkResetFences(device_.device, 1, &fence), "Failed to reset the fence");
}

bool vkn::Signal::signaled()
{
	if (vkGetFenceStatus(device_.device, fence) == VK_SUCCESS)
	{
#ifndef NDEBUG
		signaled_ = true;
#endif
		return true;
	}
	else
	{
#ifndef NDEBUG
		signaled_ = false;
#endif
		return false;
	}
}

void vkn::Signal::waitForSignal(const uint64_t timeout) const
{
	vkWaitForFences(device_.device, 1, &fence, true, timeout);
}
