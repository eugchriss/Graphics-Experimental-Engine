#pragma once
#include "vulkan/vulkan.hpp"
#include "Instance.h"
#include <fstream>
#include <string>
#include <vector>
namespace vkn
{
	class Gpu
	{
	public:
		Gpu(const VkPhysicalDevice device_);
		const VkPhysicalDeviceFeatures2& enabledFeatures();
		const VkPhysicalDeviceProperties properties() const;
		const VkPhysicalDeviceMemoryProperties memoryProperties() const;
		friend std::ostream& operator << (std::ostream& os, const Gpu& gpu);
		static const std::vector<Gpu> getAvailbleGpus(const vkn::Instance& instance);
		const float timeStamp()const;
		VkPhysicalDevice device{ VK_NULL_HANDLE };
	private:
		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures_{};

		const std::string typeToString(const VkPhysicalDeviceType type) const;
	};
}

