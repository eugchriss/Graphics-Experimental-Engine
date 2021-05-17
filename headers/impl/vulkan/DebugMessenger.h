#pragma once
#include <memory>
#include <string>
#include <queue>
#include "Instance.h"
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class DebugMessenger
	{
	public:
		DebugMessenger(std::shared_ptr<vkn::Instance>& instance, const VkDebugUtilsMessageSeverityFlagsEXT severity, const VkDebugUtilsMessageTypeFlagsEXT type);
		DebugMessenger(DebugMessenger&& other);
		~DebugMessenger();
	private:
		std::shared_ptr<vkn::Instance> instance_;
		VkDebugUtilsMessengerEXT messenger_{ VK_NULL_HANDLE };
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severityFlags, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		static std::string severityToString(const VkDebugUtilsMessageSeverityFlagBitsEXT severity);
		static std::string typeToString(const VkDebugUtilsMessageTypeFlagsEXT type);

	};
}