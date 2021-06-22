#pragma once
#include <string>
#include <vector>

#include "Instance.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		struct Gpu
		{		
			Gpu(const VkPhysicalDevice device_);
			const VkPhysicalDeviceProperties properties() const;
			const VkPhysicalDeviceMemoryProperties memoryProperties() const;
			static const std::vector<Gpu> getAvailbleGpus(const vkn::Instance& instance);
			const float timeStamp()const;
			VkPhysicalDevice device{ VK_NULL_HANDLE };
		};
	}
}
