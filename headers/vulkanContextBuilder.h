#pragma once
#include <string>
#include <vector>
#include "vulkanContext.h"
#include "Window.h"

namespace vkn
{
	class ContextBuilder
	{
	public:
		ContextBuilder(const VkDebugUtilsMessageSeverityFlagsEXT serverity, const VkDebugUtilsMessageTypeFlagsEXT type);
		ContextBuilder(ContextBuilder&&) = default;
		Context build(const gee::Window& window) const;
		void addInstanceLayer(const std::string& layerName);
		void addDeviceLayer(const std::string& layerName);
		void addInstanceExtention(const std::string& extensionName);
		void addDeviceExtention(const std::string& extensionName);
		void setQueueCount(const uint32_t count);
		void addQueueFlag(const VkQueueFlagBits queueType);

	private:
		VkDebugUtilsMessageSeverityFlagsEXT severity_;
		VkDebugUtilsMessageTypeFlagsEXT messageType_;
		std::vector<const char*> instanceLayers_;
		std::vector<const char*> deviceLayers_;
		std::vector<const char*> instanceExtensions_;
		std::vector<const char*> deviceExtensions_;
		VkQueueFlags queueType_{};
		uint32_t queueCount_{};
	};
}