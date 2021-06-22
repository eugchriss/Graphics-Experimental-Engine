#include <algorithm>
#include <cassert>
#include <iostream>

#include "../../headers/impl/vulkan/Instance.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"
#include "GLFW/glfw3.h"

using namespace gee;
vkn::Instance::Instance(const std::vector<std::string>& requestedLayers, const std::vector<std::string>& extensions)
{
	uint32_t count{};
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> layersProps(count);
	vkEnumerateInstanceLayerProperties(&count, std::data(layersProps));
	for (auto i = 0u; i < count; ++i)	
		availableLayers.push_back(layersProps[i].layerName);

	std::vector<const char*> layers;
	for (const auto& layer : requestedLayers)
	{
		layers.emplace_back(layer.c_str());
	}
	//checkLayerPresence(layers);

	std::vector<const char*> ext;
	for (auto& e : extensions)
	{
		ext.emplace_back(e.c_str());
	}
	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = nullptr;
	info.enabledExtensionCount = std::size(ext);
	info.ppEnabledExtensionNames = std::data(ext);
	info.enabledLayerCount = std::size(layers);
	info.ppEnabledLayerNames = std::data(layers);
	vkn::error_check(vkCreateInstance(&info, nullptr, &instance), "Unabled to create an instance");
}

vkn::Instance::Instance(Instance&& other)
{
	instance = other.instance;
	other.instance = VK_NULL_HANDLE;
	availableLayers = std::move(other.availableLayers);
}

vkn::Instance::~Instance()
{
	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, nullptr);
	}
}

void vkn::Instance::checkLayerPresence(const std::vector<const char*>& requested) const 
{
	for (auto layer : requested)
	{
		auto it = std::find(std::begin(availableLayers), std::end(availableLayers), layer);
		std::cout << std::distance(std::begin(availableLayers), it);
	}
}
