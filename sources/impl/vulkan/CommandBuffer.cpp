#include "../../headers/impl/vulkan/commandbuffer.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"	

#ifndef NDEBUG
void vkn::CommandBuffer::setDebugName(const std::string& name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.pNext = nullptr;
	nameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(cb_);
	nameInfo.pObjectName = name.c_str();

	context_.device->setDebugOjectName(nameInfo);
}
#endif 

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
