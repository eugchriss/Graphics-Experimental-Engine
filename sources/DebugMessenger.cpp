#include "..\headers\DebugMessenger.h"
#include "../headers/vulkan_utils.h"
#include <iostream>

vkn::DebugMessenger::DebugMessenger(std::shared_ptr<vkn::Instance>& instance, const VkDebugUtilsMessageSeverityFlagsEXT severity, const VkDebugUtilsMessageTypeFlagsEXT type):
	instance_{instance}
{
	VkDebugUtilsMessengerCreateInfoEXT debugCI{};
	debugCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCI.pNext = nullptr;
	debugCI.flags = 0;
	debugCI.messageSeverity = severity;
	debugCI.messageType = type;
	debugCI.pfnUserCallback = debugCallback;
	debugCI.pUserData = nullptr;

	auto createDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_->instance, "vkCreateDebugUtilsMessengerEXT"));
	vkn::error_check(createDebugUtilsMessengerEXT(instance_->instance, &debugCI, nullptr, &messenger_), "Unable to create the debug report");
}

vkn::DebugMessenger::DebugMessenger(DebugMessenger&& other): instance_{other.instance_}
{
	messenger_ = other.messenger_;
	other.messenger_ = VK_NULL_HANDLE;
}

vkn::DebugMessenger::~DebugMessenger()
{
	if (messenger_ != VK_NULL_HANDLE)
	{
		auto destroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_->instance, "vkDestroyDebugUtilsMessengerEXT"));
		destroyDebugUtilsMessengerEXT(instance_->instance, messenger_, nullptr);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkn::DebugMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (severity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT && type != VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
	{
		std::cout << typeToString(type) << " " << severityToString(severity) << "\n";
		if (pCallbackData->objectCount > 0)
		{
			for (auto i = 0u; i < pCallbackData->objectCount; ++i)
			{
				if (pCallbackData->pObjects[i].pObjectName)
				{
					std::cout << pCallbackData->pObjects[i].pObjectName << "\n";
				}
				else
				{
					std::cout << pCallbackData->pObjects[i].objectType << "\n";
				}
			}
			std::cout << "\t" << pCallbackData->pMessage << "\n\n";
		}
		else
		{
			std::cout << pCallbackData->pMessage << "\n\n";
		}
	}
	return VK_FALSE;
}

std::string vkn::DebugMessenger::severityToString(const VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		return "[VERBOSE]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		return "[INFO]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		return "[WARNING]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		return "[ERROR]";
		break;
	default:
		return "[UNKNOWN]";
		break;
	}
}

std::string vkn::DebugMessenger::typeToString(const VkDebugUtilsMessageTypeFlagsEXT type)
{
	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		return ">GENERAL<";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		return ">VALIDATION<";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		return ">PERFORMANCE<";
		break;
	default:
		return ">UNKNOWN<";
		break;
	}
}