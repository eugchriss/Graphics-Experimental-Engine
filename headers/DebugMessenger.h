#pragma once
#include "vulkan/vulkan.hpp"
#include "Instance.h"
#include <queue>
#include <string>

namespace vkn
{
	class DebugMessenger
	{
	public:
		DebugMessenger(const vkn::Instance& instance, const VkDebugUtilsMessageSeverityFlagsEXT severity, const VkDebugUtilsMessageTypeFlagsEXT type);
		~DebugMessenger();
		const std::string& lastMessage() const;
	private:
		const VkInstance instance_;
		VkDebugUtilsMessengerEXT messenger_{ VK_NULL_HANDLE };
		static std::queue<std::string> messages_;
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severityFlags, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		static std::string severityToString(const VkDebugUtilsMessageSeverityFlagBitsEXT severity);
		static std::string typeToString(const VkDebugUtilsMessageTypeFlagsEXT type);

	};
}