#pragma once
#include "DeviceMemory.h"
#include "vulkanContext.h"
#include "vulkan_utils.h"

#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		class Queue;
		class Image;
		class Buffer
		{
		public:
			Buffer(Context& context, const VkBufferUsageFlags usage, const VkDeviceSize size);
			Buffer(Buffer&&);
			~Buffer();
			const VkDeviceSize getMemorySize() const;
			void bind(DeviceMemory& memory);
			void moveTo(Queue& queue, DeviceMemory& memory);
			const float rawContentAt(const VkDeviceSize offset) const;
			const std::vector<float> rawContent() const;
			template<class T>
			VkDeviceSize add(const T& data);

			template<class T>
			void update(const VkDeviceSize offset, const T& data);

			void update(const VkDeviceSize offset, void* datas, const size_t size);

			VkBuffer buffer{ VK_NULL_HANDLE };

		private:
			Context& context_;
			Observer_ptr<vkn::DeviceMemory> memory_{};
			VkDeviceSize memoryOffset_{};
			VkDeviceSize offset_{};
			VkDeviceSize size_{};
			VkBufferUsageFlags usage_;
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
}