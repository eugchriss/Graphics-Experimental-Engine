#pragma once
#include "vulkan/vulkan.hpp"
#include "Instance.h"
#include "DebugMessenger.h"
#include "Gpu.h"
#include "Device.h"
#include "QueueFamily.h"

namespace vkn
{
	class Context
	{
	public:
		Context(Context&&) = default;
		~Context();
		vkn::Instance instance;
		vkn::DebugMessenger debugMessenger;
		VkSurfaceKHR surface;
		vkn::Gpu gpu;
		vkn::QueueFamily& queueFamily;
		vkn::Device device;
	private:
		friend class ContextBuilder;
		Context(vkn::Instance&& _instance, vkn::DebugMessenger&& _debugMessenger, const VkSurfaceKHR _surface, vkn::Gpu&& _gpu, vkn::QueueFamily&& _queueFamily, vkn::Device&& _device);
	};
}