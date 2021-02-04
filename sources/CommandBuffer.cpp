#include "../headers/commandbuffer.h"
#include "../headers/vulkan_utils.h"	

#ifndef NDEBUG
void vkn::CommandBuffer::setDebugName(const std::string& name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.pNext = nullptr;
	nameInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(cb_);
	nameInfo.pObjectName = name.c_str();

	context_.get().device->setDebugOjectName(nameInfo);
}
#endif 
void vkn::CommandBuffer::begin(const VkCommandBufferUsageFlags usage)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = usage;
	beginInfo.pInheritanceInfo = nullptr;

	vkn::error_check(vkBeginCommandBuffer(cb_, &beginInfo), "Failed to begin the command buffer");
	isRecording_ = true;
}

void vkn::CommandBuffer::end()
{
	vkn::error_check(vkEndCommandBuffer(cb_), "Failed to end the command buffer");
	isRecording_ = false;
}

VkCommandBuffer vkn::CommandBuffer::commandBuffer() const
{
	return cb_;
}

bool vkn::CommandBuffer::isComplete()
{
	return complete_.signaled();
}

vkn::Signal& vkn::CommandBuffer::completeSignal()
{
	return complete_;
}

vkn::CommandBuffer::CommandBuffer(vkn::Context& context, const VkCommandBuffer cb) :context_{std::ref(context) }, cb_{ cb }, complete_{ context_, false }
{
}

const bool vkn::CommandBuffer::isRecording() const
{
	return isRecording_;
}
