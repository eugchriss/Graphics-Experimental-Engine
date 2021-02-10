#include "..\headers\vulkanContext.h"

vkn::Context::Context(std::shared_ptr<vkn::Instance>& _instance, std::unique_ptr<vkn::DebugMessenger>& _debugMessenger, const VkSurfaceKHR _surface, std::shared_ptr<vkn::Gpu>& _gpu, std::shared_ptr<vkn::QueueFamily>& _queueFamily, std::unique_ptr<vkn::Device>& _device) :
	instance{ _instance },
	debugMessenger{std::move(_debugMessenger) },
	surface{ _surface },
	gpu{ _gpu },
	queueFamily{std::move(_queueFamily)},
	device{ std::move(_device) }
{
	graphicsQueue = queueFamily->getQueue(*device);
	transferQueue = queueFamily->getQueue(*device);
}

vkn::Context::Context(Context&& other) : 
	instance{ std::move(other.instance) },
	debugMessenger{ std::move(other.debugMessenger) },
	gpu{ std::move(other.gpu) },
	device{ std::move(other.device) },
	queueFamily{ std::move(other.queueFamily) }
{
	graphicsQueue = std::move(other.graphicsQueue);
	transferQueue = std::move(other.transferQueue);
	surface = other.surface;
	other.surface = VK_NULL_HANDLE;
}

vkn::Context::~Context()
{
	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance->instance, surface, nullptr);
	}
}

const bool vkn::Context::pushDescriptorEnabled() const
{
	return device->pushDescriptorEnabled();
}
