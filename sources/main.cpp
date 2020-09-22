#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <vector>
int main()
{
	glfwInit();
	if (glfwVulkanSupported())
	{
		std::cout << "vulkan supported\n";
	}
	else
	{
		std::cout << "vulkan is not supported \n";
		return 0;
	}
	VkInstance instance;
	VkInstanceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = nullptr;
	vkCreateInstance(&info, nullptr, &instance);

	uint32_t count{};
	vkEnumeratePhysicalDevices(instance, &count, nullptr);
	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(instance, &count, std::data(devices));
	std::cout << "Available graphics cards:\n";
	for (const auto device : devices)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		std::cout << "\t-" << props.deviceName << "\n";
	}
	std::cin.get();
	return 0;
}