#include "../../headers/impl/vulkan/Device.h"
#include "../../headers/impl/vulkan/QueueFamily.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;
vkn::Device::Device(vkn::Gpu& gpu,const std::vector<std::string>& requestedExtensions, const vkn::QueueFamily& queueFamily)
{
	std::vector<const char*> extensions;
	for (const auto& extension : requestedExtensions)
	{
		extensions.emplace_back(extension.c_str());
	}
	VkDeviceCreateInfo deviceCI{};
	auto queueInfo = queueFamily.info();
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = nullptr;
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &queueInfo;
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = std::size(extensions);
	deviceCI.ppEnabledExtensionNames = std::data(extensions);
	deviceCI.pEnabledFeatures = nullptr;

	vkn::error_check(vkCreateDevice(gpu.device, &deviceCI, nullptr, &device), "unable to create a logical device");
}

vkn::Device::Device(Device&& other)
{
	device = other.device;
	other.device = VK_NULL_HANDLE;
}

vkn::Device::~Device()
{	
	if (device != VK_NULL_HANDLE)
	{
		idle();
		vkDestroyDevice(device, nullptr);
	}
}
void vkn::Device::idle()
{
	vkn::error_check(vkDeviceWaitIdle(device), "Unabled to idle the device");
}
