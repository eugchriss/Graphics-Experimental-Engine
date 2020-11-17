#include "../headers/DeviceMemory.h"
#include "../headers/vulkan_utils.h"
#include <cassert>

uint32_t vkn::DeviceMemory::allocationCount = 0;

vkn::DeviceMemory::DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const VkMemoryPropertyFlagBits property, const VkDeviceSize size) : device_{ device }, size_{ size }, location_{ static_cast<VkMemoryPropertyFlags>(property)}
{
	auto memProps = gpu.memoryProperties();
	bool found{ false };
	for (auto i = 0u; i < memProps.memoryTypeCount; ++i)
	{
		if ((memProps.memoryTypes[i].propertyFlags & property) == property)
		{
			if (size_ > 0)
			{
				if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > size_)
				{
					found = true;
					memoryIndex_ = i;
					break;
				}
			}
			else
			{
				if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > 0)
				{
					found = true;
					size_ = 0.2 * memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size;
					memoryIndex_ = i;
					break;
				}
			}
		}
	}
	assert(found && "Unabled to find a suitale memory");

	VkMemoryAllocateInfo memoryInfo{};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.pNext = nullptr;
	memoryInfo.allocationSize = size_;
	memoryInfo.memoryTypeIndex = memoryIndex_;

	assert(allocationCount <= gpu.properties().limits.maxMemoryAllocationCount && "The application reached the maximum allocation count");
	vkn::error_check(vkAllocateMemory(device_.device, &memoryInfo, nullptr, &memory_), "Failed to allocate the memory");
	++allocationCount;
}

vkn::DeviceMemory::DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const VkMemoryPropertyFlagBits property, const uint32_t memoryTypeBits, const VkDeviceSize size) : device_{ device }, size_{ size }, location_{ static_cast<VkMemoryPropertyFlags>(property) }
{
	auto memProps = gpu.memoryProperties();
	bool found{ false };
	for (auto i = 0u; i < memProps.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) == (1 << i) && (memProps.memoryTypes[i].propertyFlags & property) == property)
		{
			if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > size_)
			{
				found = true;
				memoryIndex_ = i;
				break;
			}
		}
	}
	assert(found && "Unabled to find a suitale memory");

	VkMemoryAllocateInfo memoryInfo{};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.pNext = nullptr;
	memoryInfo.allocationSize = size_;
	memoryInfo.memoryTypeIndex = memoryIndex_;

	assert(allocationCount <= gpu.properties().limits.maxMemoryAllocationCount && "The application reached the maximum allocation count");
	vkn::error_check(vkAllocateMemory(device_.device, &memoryInfo, nullptr, &memory_), "Failed to allocate the memory");
	++allocationCount;
}

vkn::DeviceMemory::DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const uint32_t memoryTypeBits, const VkDeviceSize size) : device_{ device }, size_{ size }
{
	auto memProps = gpu.memoryProperties();
	bool found{ false };
	for (auto i = 0u; i < memProps.memoryTypeCount; ++i)
	{
		if ((memoryTypeBits & (1 << i)) == (1 << i) && (memProps.memoryTypes[i].propertyFlags != 0))
		{
			if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > size_)
			{
				found = true;
				memoryIndex_ = i;
				location_ = memProps.memoryTypes[i].propertyFlags;
				break;
			}
		}
	}
	assert(found && "Unabled to find a suitale memory");

	VkMemoryAllocateInfo memoryInfo{};
	memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryInfo.pNext = nullptr;
	memoryInfo.allocationSize = size_;
	memoryInfo.memoryTypeIndex = memoryIndex_;

	assert(allocationCount <= gpu.properties().limits.maxMemoryAllocationCount && "The application reached the maximum allocation count");
	vkn::error_check(vkAllocateMemory(device_.device, &memoryInfo, nullptr, &memory_), "Failed to allocate the memory");
	++allocationCount;
}

vkn::DeviceMemory::DeviceMemory(DeviceMemory&& other): device_{other.device_}
{
	memory_ = other.memory_;
	size_ = other.size_;
	memoryIndex_ = other.memoryIndex_;
	offset_ = other.offset_;
	location_ = other.location_;

	other.memory_ = VK_NULL_HANDLE;
}

vkn::DeviceMemory::~DeviceMemory()
{
	if (memory_ != VK_NULL_HANDLE)
	{
		vkFreeMemory(device_.device, memory_, nullptr);
		--allocationCount;
	}
}

const VkDeviceSize vkn::DeviceMemory::bind(const VkBuffer buffer)
{
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(device_.device, buffer, &memRequirements);

	assert(size_ >= memRequirements.size && "The memory capacity is not high enough for the buffer");
	assert(((memRequirements.memoryTypeBits & (1 << memoryIndex_)) == (1 << memoryIndex_)) && "The buffer doesn t match this memory index");

	checkAlignment(memRequirements.alignment);
	vkn::error_check(vkBindBufferMemory(device_.device, buffer, memory_, offset_), "Unabled to bind the buffer and the memory");
	auto offset = offset_;
	offset_ += memRequirements.size;
	return offset;
}

const VkDeviceSize vkn::DeviceMemory::bind(const VkImage image)
{
	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(device_.device, image, &memRequirements);

	assert(size_ >= memRequirements.size && "The memory capacity is not high enough for the image");
	assert((memRequirements.memoryTypeBits & memoryIndex_) == memoryIndex_ && "The image doesn t match this memory index");

	checkAlignment(memRequirements.alignment);
	vkn::error_check(vkBindImageMemory(device_.device, image, memory_, offset_), "Unabled to bind the image and the memory");
	
	auto offset = offset_;
	offset_ += memRequirements.size;
	return offset;
}

const std::vector<unsigned char> vkn::DeviceMemory::rawContent(const VkDeviceSize offset, const VkDeviceSize size) const
{
	if (((location_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) || ((location_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || ((location_ & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
	{
		std::vector<unsigned char> datas(size);
		void* ptr;
		vkn::error_check(vkMapMemory(device_.device, memory_, offset, size, 0, &ptr), "Unabled to map memory to vulkan");
		memcpy(std::data(datas), ptr, size);
		vkUnmapMemory(device_.device, memory_);
		return std::move(datas);
	}
	else
	{
		throw std::runtime_error{ "Attempting to map a non mappable memory type" };
	}
}

void vkn::DeviceMemory::checkAlignment(const VkDeviceSize alignment)
{
	if (offset_ % alignment != 0)
	{
		offset_ += alignment - (offset_ % alignment);
	}
}