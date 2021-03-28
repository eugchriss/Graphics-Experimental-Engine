#include "..\headers\Swapchain.h"
#include "../headers/vulkan_utils.h"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <cassert>

vkn::Swapchain::Swapchain(Context& context): context_{context}
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vkn::error_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context_.gpu->device, context_.surface, &surfaceCapabilities), "Failed to get surface capabilities");

	auto format = getSurfaceFormat(*context_.gpu, context_.surface);
	swapchainInfo_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo_.pNext = nullptr;
	swapchainInfo_.flags = 0;
	swapchainInfo_.surface = context_.surface;
	swapchainInfo_.minImageCount = surfaceCapabilities.maxImageCount;
	if (swapchainInfo_.minImageCount > 2)
	{
		swapchainInfo_.minImageCount = 2;
	}
	swapchainInfo_.imageFormat = format.format;
	swapchainInfo_.imageColorSpace = format.colorSpace;
	swapchainInfo_.imageExtent = surfaceCapabilities.currentExtent;
	swapchainInfo_.imageArrayLayers = 1;
	swapchainInfo_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo_.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo_.preTransform = surfaceCapabilities.currentTransform;
	swapchainInfo_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo_.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainInfo_.clipped = VK_TRUE;
	swapchainInfo_.oldSwapchain = VK_NULL_HANDLE;
	vkn::error_check(vkCreateSwapchainKHR(context_.device->device, &swapchainInfo_, nullptr, &swapchain_), "Unable to create the swapchain");

	retrieveTargets();
}

vkn::Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(context_.device->device, swapchain_, nullptr);
}

void vkn::Swapchain::resize(const VkExtent2D& extent)
{
	swapchainInfo_.imageExtent = extent;
	swapchainInfo_.oldSwapchain = swapchain_;
	vkn::error_check(vkCreateSwapchainKHR(context_.device->device, &swapchainInfo_, nullptr, &swapchain_), "Unable to REcreate the swapchain");
	retrieveTargets();
}

const VkExtent2D vkn::Swapchain::extent() const
{
	return swapchainInfo_.imageExtent;
}

void vkn::Swapchain::swapBuffers(const uint64_t timeout)
{
	Fence ready{ context_, false };
	vkn::error_check(vkAcquireNextImageKHR(context_.device->device, swapchain_, timeout, VK_NULL_HANDLE, ready(), &availableImageIndex_), "Failed to acquire the next image");
	ready.wait();
}

const VkPresentInfoKHR vkn::Swapchain::imagePresentInfo(Semaphore& waitOnSemaphore) const
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitOnSemaphore();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain_;
	presentInfo.pImageIndices = &availableImageIndex_;
	presentInfo.pResults = nullptr;

	return presentInfo;
}

std::vector<vkn::RenderTarget>& vkn::Swapchain::renderTargets()
{
	return renderTargets_;
}

const VkFormat vkn::Swapchain::imageFormat() const
{
	return swapchainInfo_.imageFormat;
}

const VkSurfaceFormatKHR vkn::Swapchain::getSurfaceFormat(vkn::Gpu& gpu, const VkSurfaceKHR& surface) const
{
	uint32_t count{};
	vkn::error_check(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &count, nullptr), "Failed to get surface format");
	std::vector<VkSurfaceFormatKHR> formats(count);
	vkn::error_check(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.device, surface, &count, std::data(formats)), "Failed to get surface format");
	auto result = std::find_if(std::begin(formats), std::end(formats), [](auto format) { return format.format == VK_FORMAT_B8G8R8A8_SRGB; });
	if (result == std::end(formats))
	{
		assert(!std::empty(formats) && "There is no available format for this surface");
		return formats[0];
	}
	else
	{
		return *result;
	}
}

void vkn::Swapchain::retrieveTargets()
{
	uint32_t count;
	vkn::error_check(vkGetSwapchainImagesKHR(context_.device->device, swapchain_, &count, nullptr), "Unabled to get the swapchain images count");
	std::vector<VkImage> images(count);
	vkn::error_check(vkGetSwapchainImagesKHR(context_.device->device, swapchain_, &count, std::data(images)), "Unabled to get the swapchain images");

	renderTargets_.clear();
	for (auto image : images)
	{
		renderTargets_.emplace_back(context_, image, swapchainInfo_.imageFormat);
		//TODO: copy all metadata of the old renderTarget to the new one
	}
}
