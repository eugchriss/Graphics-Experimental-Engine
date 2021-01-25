#include "..\headers\Device.h"
#include "../headers/vulkan_utils.h"
#include "../headers/QueueFamily.h"

vkn::Device::Device(vkn::Gpu& gpu,const std::vector<std::string>& requestedExtensions, const vkn::QueueFamily& queueFamily)
{
	std::vector<const char*> extensions;
	for (const auto& extension : requestedExtensions)
	{
		extensions.emplace_back(extension.c_str());
	}
	auto features = gpu.enabledFeatures();
	VkDeviceCreateInfo deviceCI{};
	auto queueInfo = queueFamily.info();
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = &features;
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &queueInfo;
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = std::size(extensions);
	deviceCI.ppEnabledExtensionNames = std::data(extensions);
	deviceCI.pEnabledFeatures = nullptr;

	vkn::error_check(vkCreateDevice(gpu.device, &deviceCI, nullptr, &device), "unable to create a logical device");

	vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
}

vkn::Device::Device(Device&& other)
{
	device = other.device;
	other.device = VK_NULL_HANDLE;
	vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
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
#ifndef NDEBUG
void vkn::Device::setDebugOjectName(const VkDebugUtilsObjectNameInfoEXT& nameInfo)
{
	vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}
#endif 
