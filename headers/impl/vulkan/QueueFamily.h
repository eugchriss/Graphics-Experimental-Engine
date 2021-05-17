#pragma once
#include <memory>
#include <vector>

#include "Gpu.h"
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class Device;
	class Queue;
	class QueueFamily
	{
	public:
		QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, const uint32_t queueCount);
		QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, const std::vector<float>& queuePriorities);
		QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, VkSurfaceKHR surface, const uint32_t queueCount);
		QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, VkSurfaceKHR surface, const std::vector<float>& queuePriorities);
		QueueFamily(QueueFamily&&) = default;
		const uint32_t familyIndex() const;
		const VkDeviceQueueCreateInfo info() const;
		std::unique_ptr<Queue> getQueue(const vkn::Device& device);
		uint32_t timestampValidBits() const;
	private:
		uint32_t familyIndex_{};
		VkQueueFlags familyType_{};
		std::vector<float> queuePriorities_;
		uint32_t queueIndex_{};
		uint32_t timestampValidBits_{};
	};
}