#include "../../headers/impl/vulkan/Buffer.h"
#include "../../headers/impl/vulkan/CommandPool.h"

vkn::Buffer::Buffer(Context& context, const VkBufferUsageFlags usage, const VkDeviceSize size) : context_{ context }, size_{size}, usage_{usage}
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = size_;
	bufferInfo.usage = usage_;
	vkn::error_check(vkCreateBuffer(context_.device->device, &bufferInfo, nullptr, &buffer), "Failed to create the buffer");
}

vkn::Buffer::Buffer(Buffer&& other): context_{other.context_}
{
	buffer = other.buffer;
	offset_ = other.offset_;
	size_ = other.size_;
	usage_ = other.usage_;
	memoryOffset_ = other.memoryOffset_;
	memory_ = std::move(other.memory_);
	other.buffer = VK_NULL_HANDLE;
}

vkn::Buffer::~Buffer()
{
	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(context_.device->device, buffer, nullptr);
	}
}

const VkDeviceSize vkn::Buffer::getMemorySize() const
{
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(context_.device->device, buffer, &memRequirements);
	return memRequirements.size;
}

void vkn::Buffer::bind(DeviceMemory& memory)
{
	memory_ = Observer_ptr<vkn::DeviceMemory>{ memory };
	memoryOffset_ = memory.bind(buffer);
}

void vkn::Buffer::moveTo(Queue& queue, DeviceMemory& memory)
{
	Buffer dst{ context_, usage_ | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size_ };
	dst.bind(memory);
	VkBufferCopy copy{};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = size_;

	vkn::CommandPool cbPool{ context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto& cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	vkCmdCopyBuffer(cb.commandBuffer(), buffer, dst.buffer, 1, &copy);
	cb.end();

	auto& completedFence = queue.submit(cb);
	completedFence->wait();

	memory_ = Observer_ptr<vkn::DeviceMemory>{ memory };
	vkDestroyBuffer(context_.device->device, buffer, nullptr);
	buffer = dst.buffer;
	offset_ = dst.offset_;
	memoryOffset_ = dst.memoryOffset_;
	dst.buffer = VK_NULL_HANDLE;
}

const float vkn::Buffer::rawContentAt(const VkDeviceSize offset) const
{
	return memory_->rawContentAt(memoryOffset_ + offset);
}

const std::vector<float> vkn::Buffer::rawContent() const
{
	return memory_->rawContent(memoryOffset_, size_);
}
