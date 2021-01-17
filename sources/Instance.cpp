#include "..\headers\Instance.h"
#include "../headers/vulkan_utils.h"
#include "GLFW/glfw3.h"
#include <iostream>
#include <algorithm>
#include <cassert>

vkn::Instance::Instance(const std::vector<const char*>& requestedLayers)
{
	uint32_t count{};
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> layersProps(count);
	vkEnumerateInstanceLayerProperties(&count, std::data(layersProps));
	for (auto i = 0u; i < count; ++i)	
		availableLayers.push_back(layersProps[i].layerName);

	count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensionsProps(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, std::data(extensionsProps));
	std::vector<const char*> extensions;
	for (auto i = 0u; i < count; ++i)
		extensions.push_back(extensionsProps[i].extensionName);

	//checkLayerPresence(requestedLayers);

	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = nullptr;
	info.enabledExtensionCount = std::size(extensions);
	info.ppEnabledExtensionNames = std::data(extensions);
	info.enabledLayerCount = std::size(requestedLayers);
	info.ppEnabledLayerNames = std::data(requestedLayers);
	vkn::error_check(vkCreateInstance(&info, nullptr, &instance), "Unabled to create an instance");
}

vkn::Instance::~Instance()
{
	vkDestroyInstance(instance, nullptr);
}

void vkn::Instance::checkLayerPresence(const std::vector<const char*>& requested) const 
{
	for (auto layer : requested)
	{
		auto it = std::find(std::begin(availableLayers), std::end(availableLayers), layer);
		std::cout << std::distance(std::begin(availableLayers), it);
	}
}
