#include "../headers/VulkanContext.h"
#include "../headers/impl/vulkan/vulkanContextBuilder.h"

gee::VulkanContext::VulkanContext(gee::Window& window)
{
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vkn::ContextBuilder contextBuilder{ severityFlags, messageTypeFlags };
	//#ifndef NDEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	contextBuilder.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	contextBuilder.addInstanceLayer("VK_LAYER_LUNARG_monitor");
	//#endif
	for (const auto& instanceExtension : instanceExtensions)
	{
		contextBuilder.addInstanceExtention(instanceExtension);
	}
	contextBuilder.addDeviceExtention("VK_KHR_swapchain");
	contextBuilder.addQueueFlag(VK_QUEUE_GRAPHICS_BIT);
	contextBuilder.addQueueFlag(VK_QUEUE_TRANSFER_BIT);
	contextBuilder.setQueueCount(2);

	context = std::make_unique<vkn::Context>(std::move(contextBuilder.build(window)));
}
