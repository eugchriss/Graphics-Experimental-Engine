#include "../headers/signal.h"
#include "../headers/vulkan_utils.h"


vkn::Fence::Fence(Context& context, const bool signaled): context_{context}
{
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	vkn::error_check(vkCreateFence(context_.device->device, &fenceCI, nullptr, &fence_), "Unable to create the rendering finished fence");
}

vkn::Fence::Fence(Fence&&other): context_{other.context_}
{
	fence_ = other.fence_;
	other.fence_ = VK_NULL_HANDLE;
}

vkn::Fence::~Fence()
{
	if (fence_ != VK_NULL_HANDLE)
	{
		vkDestroyFence(context_.device->device, fence_, nullptr);
	}
}

VkFence& vkn::Fence::operator()()
{
	return fence_;
}

void vkn::Fence::wait(const uint64_t timeout) const
{
	vkWaitForFences(context_.device->device, 1, &fence_, true, timeout);
}

bool vkn::Fence::signaled() const
{
	return vkGetFenceStatus(context_.device->device, fence_) == VK_SUCCESS;
}

void vkn::Fence::reset()
{
	vkn::error_check(vkResetFences(context_.device->device, 1, &fence_), "Failed to reset the fence");
}

vkn::Semaphore::Semaphore(Context& context): context_{context}
{
	VkSemaphoreCreateInfo semaphoreCI{};
	semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCI.pNext = nullptr;
	semaphoreCI.flags = 0;

	vkn::error_check(vkCreateSemaphore(context.device->device, &semaphoreCI, nullptr, &semaphore_), "Unable to create the rendering-finshed semaphore");
}

vkn::Semaphore::Semaphore(Semaphore&& other) : context_{ other.context_ }
{
	semaphore_ = other.semaphore_;
	other.semaphore_ = VK_NULL_HANDLE;
}

vkn::Semaphore::~Semaphore()
{
	if (semaphore_ != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(context_.device->device, semaphore_, nullptr);
	}
}

VkSemaphore& vkn::Semaphore::operator()()
{
	return semaphore_;
}
