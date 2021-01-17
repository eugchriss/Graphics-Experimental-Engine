#include "..\headers\Device.h"
#include "../headers/vulkan_utils.h"
#include "../headers/QueueFamily.h"

vkn::Device::Device(const vkn::Gpu& gpu,const std::vector<const char*>& requestedExtensions, const vkn::QueueFamily& queueFamily)
{
	VkDeviceCreateInfo deviceCI{};
	auto queueInfo = queueFamily.info();
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.pNext = &gpu.enabledFeatures();
	deviceCI.flags = 0;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &queueInfo;
	deviceCI.enabledLayerCount = 0;
	deviceCI.ppEnabledLayerNames = nullptr;
	deviceCI.enabledExtensionCount = std::size(requestedExtensions);
	deviceCI.ppEnabledExtensionNames = std::data(requestedExtensions);
	deviceCI.pEnabledFeatures = nullptr;

	vkn::error_check(vkCreateDevice(gpu.device, &deviceCI, nullptr, &device), "unable to create a logical device");

	vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
}

vkn::Device::~Device()
{
	vkDestroyDevice(device, nullptr);
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
