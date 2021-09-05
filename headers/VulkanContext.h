#pragma once
#include <memory>

#include "Window.h"
#include "impl/vulkan/vulkanContext.h"

namespace gee
{
	struct VulkanContext
	{
		VulkanContext(gee::Window& window);
		std::unique_ptr<vkn::Context> context;
	};
}
