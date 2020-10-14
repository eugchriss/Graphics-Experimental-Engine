#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "DeviceMemory.h"
#include "../headers/vulkan_utils.h"
namespace vkn
{
	class Buffer
	{
	public:
		Buffer(const vkn::Device& device, const VkBufferUsageFlags usage, const VkDeviceSize size);
		Buffer(Buffer&&);
		~Buffer();
		const VkDeviceSize getMemorySize() const;
		void bind(DeviceMemory& memory);

		template<class T>
		VkDeviceSize add(const T& data);

		template<class T>
		void update(const VkDeviceSize offset, const T& data);

		VkBuffer buffer{ VK_NULL_HANDLE };

	private:
		Observer_ptr<vkn::DeviceMemory> memory_{};
		VkDeviceSize memoryOffset_{};
		VkDeviceSize offset_{};
		const vkn::Device& device_;
	};

	template<class T>
	inline VkDeviceSize Buffer::add(const T& datas)
	{
		VkDeviceSize size;
		if constexpr (is_array<T>::value)
		{
			size = std::size(datas) * sizeof(datas[0]);
			memory_->update(memoryOffset_ + offset_, std::data(datas), size);
		}
		else
		{
			size = sizeof(datas);
			memory_->update(memoryOffset_ + offset_, &datas, size);
		}

		auto offset = offset_;
		offset_ += size;
		return offset;
	}

	template<class T>
	inline void Buffer::update(const VkDeviceSize offset, const T& datas)
	{
		if constexpr (is_array<T>::value)
		{
			memory_->update(memoryOffset_ + offset, std::data(datas), std::size(datas) * sizeof(datas[0]));
		}
		else
		{
			memory_->update(memoryOffset_ + offset, &datas, sizeof(datas));

		}
	}
}