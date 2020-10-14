#include "../headers/Buffer.h"

vkn::Buffer::Buffer(const vkn::Device& device, const VkBufferUsageFlags usage, const VkDeviceSize size) : device_{ device }
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.flags = 0;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	vkn::error_check(vkCreateBuffer(device_.device, &bufferInfo, nullptr, &buffer), "Failed to create the buffer");
}

vkn::Buffer::Buffer(Buffer&& other): device_{other.device_}
{
	buffer = other.buffer;
	offset_ = other.offset_;
	memoryOffset_ = other.memoryOffset_;

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