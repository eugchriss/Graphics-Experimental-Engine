#pragma once
#include <vector>
#include "Gpu.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		class QueueFamily;
		class Device
		{
		public:
			Device(vkn::Gpu& gpu, const std::vector<std::string>& requestedExtensions, const vkn::QueueFamily& queueFamily);
			Device(Device&& other);
			~Device();
			void idle();

			VkDevice device{ VK_NULL_HANDLE };
		private:
			std::vector<const char*> availableExtensions_;
		};
	}
}
