#pragma once
#include "vulkan/vulkan.hpp"
#include "Gpu.h"
#include "Device.h"
#include <cassert>

namespace vkn
{
	class DeviceMemory
	{
	public:
		DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const VkMemoryPropertyFlagBits property, const VkDeviceSize size = 0);
		DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const VkMemoryPropertyFlagBits property, const uint32_t memoryTypeBits, const VkDeviceSize size);
		DeviceMemory(const vkn::Gpu& gpu, const vkn::Device& device, const uint32_t memoryTypeBits, const VkDeviceSize size);
		DeviceMemory(DeviceMemory&&);
		~DeviceMemory();

		const VkDeviceSize bind(const VkBuffer buffer);
		const VkDeviceSize bind(const VkImage image);
		template<class T>
		void update(const VkDeviceSize offset, const T data, const VkDeviceSize size);
		const std::vector<float> rawContent(const VkDeviceSize offset, const VkDeviceSize size) const;
	private:
		const vkn::Device& device_;
		VkDeviceMemory memory_{ VK_NULL_HANDLE };
		VkDeviceSize size_{};
		VkDeviceSize offset_{};
		uint32_t memoryIndex_{};
		VkMemoryPropertyFlags location_;

		void checkAlignment(const VkDeviceSize aligment);

		static uint32_t allocationCount;
	};
	template<class T>
	inline void DeviceMemory::update(const VkDeviceSize offset, const T datas, const VkDeviceSize size)
	{
		if (((location_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) || ((location_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || ((location_ & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)))
		{
			assert(std::is_pointer<T>::value && "This function requires a pointer to the datas to update with");
			void* ptr;
			vkn::error_check(vkMapMemory(device_.device, memory_, offset, size, 0, &ptr), "Unabled to map memory to vulkan");
			memcpy(ptr, datas, size);
			vkUnmapMemory(device_.device, memory_);
		}
		else
		{
			throw std::runtime_error{ "Attempting to map a non mappable memory type" };
		}
	}
}