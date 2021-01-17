#include "..\headers\vulkanContext.h"

vkn::Context::Context(vkn::Instance&& _instance, vkn::DebugMessenger&& _debugMessenger, const VkSurfaceKHR _surface, vkn::Gpu&& _gpu, vkn::QueueFamily&& _queueFamily, vkn::Device&& _device) :
	instance{ _instance },
	debugMessenger{ _debugMessenger },
	surface{ _surface },
	gpu{ _gpu },
	queueFamily{_queueFamily},
	device{ _device }
{
}

vkn::Context::~Context()
{
	vkDestroySurfaceKHR(instance.instance, surface, nullptr);
}