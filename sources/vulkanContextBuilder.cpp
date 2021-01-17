#include "..\headers\vulkanContextBuilder.h"
#include "..\headers\vulkan_utils.h"
vkn::ContextBuilder::ContextBuilder(const VkDebugUtilsMessageSeverityFlagsEXT severity, const VkDebugUtilsMessageTypeFlagsEXT type): 
    severity_{severity},
    messageType_{type}
{
}

vkn::Context vkn::ContextBuilder::build(const gee::Window& window) const
{
    Instance instance{instanceLayers_};
    DebugMessenger messenger{ instance, severity_, messageType_ };
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    glfwInit();
    vkn::error_check(glfwCreateWindowSurface(instance.instance, window.window(), nullptr, &surface), "unable to create a presentable surface for the window");
    auto gpu = Gpu::getAvailbleGpus(instance)[0];
    QueueFamily queueFamily{gpu, queueType_, queueCount_};
    Device device{ gpu, deviceExtensions_, queueFamily};

    return Context{ std::move(instance), std::move(messenger), surface, std::move(gpu), std::move(queueFamily), std::move(device)};
}

void vkn::ContextBuilder::addInstanceLayer(const std::string& layerName)
{
    instanceLayers_.emplace_back(layerName.c_str());
}

void vkn::ContextBuilder::addDeviceLayer(const std::string& layerName)
{
    deviceLayers_.emplace_back(layerName.c_str());
}

void vkn::ContextBuilder::addInstanceExtention(const std::string& extensionName)
{
    instanceExtensions_.emplace_back(extensionName.c_str());
}

void vkn::ContextBuilder::addDeviceExtention(const std::string& extensionName)
{
    deviceExtensions_.emplace_back(extensionName.c_str());
}

void vkn::ContextBuilder::setQueueCount(const uint32_t count)
{
    queueCount_ = count;
}

void vkn::ContextBuilder::addQueueFlag(const VkQueueFlagBits queueType)
{
    queueType_ |= queueType;
}
