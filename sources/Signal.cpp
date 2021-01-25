#include "../headers/signal.h"
#include "../headers/vulkan_utils.h"

vkn::Signal::Signal(vkn::Context& context, bool signaled) : context_{context}
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
	vkn::error_check(vkCreateFence(context_.device->device, &fenceCI, nullptr, &fence), "Unable to create the rendering finished fence");

	VkSemaphoreCreateInfo semaphoreCI{};
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;
	
	vkn::error_check(vkCreateSemaphore(context.device->device, &semaphoreCI, nullptr, &semaphore), "Unable to create the rendering-finshed semaphore");
#ifndef NDEBUG
	signaled_ = signaled;
#endif
}

vkn::Signal::Signal(Signal&& other): context_{other.context_}
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
		vkDestroySemaphore(context_.device->device, semaphore, nullptr);
	}
	if (fence != VK_NULL_HANDLE)
	{
		vkWaitForFences(context_.device->device, 1, &fence, VK_TRUE, 500000000);
		vkDestroyFence(context_.device->device, fence, nullptr);
	}
}

void vkn::Signal::reset()
{
#ifndef NDEBUG
	signaled_ = false;
#endif
	vkn::error_check(vkResetFences(context_.device->device, 1, &fence), "Failed to reset the fence");
}

bool vkn::Signal::signaled()
{
	if (vkGetFenceStatus(context_.device->device, fence) == VK_SUCCESS)
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
	vkWaitForFences(context_.device->device, 1, &fence, true, timeout);
}
