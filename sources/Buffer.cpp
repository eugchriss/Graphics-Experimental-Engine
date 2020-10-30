#include "../headers/Buffer.h"
#include "../headers/Queue.h"
#include "../headers/CommandPool.h"

vkn::Buffer::Buffer(vkn::Device& device, const VkBufferUsageFlags usage, const VkDeviceSize size) : device_{ device }, size_{size}, usage_{usage}
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = size_;
	bufferInfo.usage = usage_;
	vkn::error_check(vkCreateBuffer(device_.device, &bufferInfo, nullptr, &buffer), "Failed to create the buffer");
}

vkn::Buffer::Buffer(Buffer&& other): device_{other.device_}
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
		vkDestroyBuffer(device_.device, buffer, nullptr);
	}
}

const VkDeviceSize vkn::Buffer::getMemorySize() const
{
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(device_.device, buffer, &memRequirements);
	return memRequirements.size;
}

void vkn::Buffer::bind(DeviceMemory& memory)
{
	memory_ = Observer_ptr<vkn::DeviceMemory>{ memory };
	memoryOffset_ = memory.bind(buffer);
}

void vkn::Buffer::moveTo(Queue& queue, DeviceMemory& memory)
{
	Buffer dst{ device_, usage_ | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size_ };
	dst.bind(memory);
	VkBufferCopy copy{};
	copy.srcOffset = 0;
	copy.dstOffset = 0;
	copy.size = size_;

	vkn::CommandPool cbPool{ device_, queue.familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	vkCmdCopyBuffer(cb.commandBuffer(), buffer, dst.buffer, 1, &copy);
	cb.end();

	vkn::Signal copyFinished{ device_ };
	queue.submit(cb, copyFinished);
	copyFinished.waitForSignal();

	memory_ = Observer_ptr<vkn::DeviceMemory>{ memory };
	vkDestroyBuffer(device_.device, buffer, nullptr);
	buffer = dst.buffer;
	offset_ = dst.offset_;
	memoryOffset_ = dst.memoryOffset_;
	dst.buffer = VK_NULL_HANDLE;
}
