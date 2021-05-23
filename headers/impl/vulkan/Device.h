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
			const bool pushDescriptorEnabled() const;
			void idle();
#ifndef NDEBUG
			void setDebugOjectName(const VkDebugUtilsObjectNameInfoEXT& nameInfo);
#endif // !NDEBUG

			VkDevice device{ VK_NULL_HANDLE };
		private:
			std::vector<const char*> availableExtensions_;
			PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
			bool pushDescriptorEnabled_{ false };
		};
	}
}
