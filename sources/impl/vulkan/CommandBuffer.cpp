#include "../../headers/impl/vulkan/commandbuffer.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"	

using namespace gee;

void vkn::CommandBuffer::attachFence(std::shared_ptr<Fence>& fence)
{
	isPendingFence_ = fence;
}

void vkn::CommandBuffer::begin(const VkCommandBufferUsageFlags usage)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = usage;
	beginInfo.pInheritanceInfo = nullptr;

	vkn::error_check(vkBeginCommandBuffer(cb_, &beginInfo), "Failed to begin the command buffer");
}

void vkn::CommandBuffer::end()
{
	vkn::error_check(vkEndCommandBuffer(cb_), "Failed to end the command buffer");

}

VkCommandBuffer vkn::CommandBuffer::commandBuffer() const
{
	return cb_;
}

const bool vkn::CommandBuffer::isPending() const
{
	return isPendingFence_ ? !isPendingFence_->signaled() : false;
}

vkn::CommandBuffer::CommandBuffer(vkn::Context& context, const VkCommandBuffer cb) :
	context_{context}, cb_{ cb },
	completedSemaphore{ context}
{
}

vkn::Context& vkn::CommandBuffer::context() const
{
	return context_;
}
