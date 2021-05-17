#pragma once
#include <string>
#include <vector>

#include "../../window.h"
#include "vulkanContext.h"

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
		std::vector<std::string> instanceLayers_;
		std::vector<std::string> deviceLayers_;
		std::vector<std::string> instanceExtensions_;
		std::vector<std::string> deviceExtensions_;
		VkQueueFlags queueType_{};
		uint32_t queueCount_{};
	};
}