#include "../../headers/impl/vulkan/vulkan_utils.h"
#include "../../headers/impl/vulkan/vulkanContextBuilder.h"

using namespace gee;
vkn::ContextBuilder::ContextBuilder(const VkDebugUtilsMessageSeverityFlagsEXT severity, const VkDebugUtilsMessageTypeFlagsEXT type): 
    severity_{severity},
    messageType_{type}
{
}

vkn::Context vkn::ContextBuilder::build(const gee::Window& window) const
{
    auto instance = std::make_shared<vkn::Instance>(instanceLayers_);
    auto messenger = std::make_unique<vkn::DebugMessenger>( instance, severity_, messageType_);
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    vkn::error_check(glfwCreateWindowSurface(instance->instance, window.window(), nullptr, &surface), "unable to create a presentable surface for the window");
    auto gpu = std::make_shared<Gpu>(Gpu::getAvailbleGpus(*instance)[0]);
    auto queueFamily = std::make_shared<vkn::QueueFamily>(*gpu, queueType_, surface, queueCount_);
    auto device = std::make_unique<vkn::Device>(*gpu, deviceExtensions_, *queueFamily);

    return Context{ instance, std::move(messenger), surface, gpu, queueFamily, std::move(device)};
}

void vkn::ContextBuilder::addInstanceLayer(const std::string& layerName)
{
    instanceLayers_.emplace_back(layerName);
}

void vkn::ContextBuilder::addDeviceLayer(const std::string& layerName)
{
    deviceLayers_.emplace_back(layerName);
}

void vkn::ContextBuilder::addInstanceExtention(const std::string& extensionName)
{
    instanceExtensions_.emplace_back(extensionName);
}

void vkn::ContextBuilder::addDeviceExtention(const std::string& extensionName)
{
    deviceExtensions_.emplace_back(extensionName);
}

void vkn::ContextBuilder::setQueueCount(const uint32_t count)
{
    queueCount_ = count;
}

void vkn::ContextBuilder::addQueueFlag(const VkQueueFlagBits queueType)
{
    queueType_ |= queueType;
}
