#include "../headers/commandbuffer.h"
#include "../headers/vulkan_utils.h"	

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

vkn::CommandBuffer::CommandBuffer(const VkCommandBuffer cb) : cb_{ cb }
{
}