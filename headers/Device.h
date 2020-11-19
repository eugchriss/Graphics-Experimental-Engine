#pragma once
#include "vulkan/vulkan.hpp"
#include <initializer_list>
#include "Gpu.h"

namespace vkn
{
	class QueueFamily;
	class Device
	{
	public:
		Device(const vkn::Gpu& gpu, const std::initializer_list<const char*>& requestedExtensions, const vkn::QueueFamily& queueFamily);
		~Device();
		void idle();
#ifndef NDEBUG
		void setDebugOjectName(const VkDebugUtilsObjectNameInfoEXT& nameInfo);
#endif // !NDEBUG

		VkDevice device{ VK_NULL_HANDLE };
	private:
		std::vector<const char*> availableExtensions_;
		PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	};
}
