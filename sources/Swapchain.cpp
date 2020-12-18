#include "..\headers\Swapchain.h"
#include "../headers/vulkan_utils.h"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <cassert>

vkn::Swapchain::Swapchain(vkn::Gpu& gpu, vkn::Device& device, const VkSurfaceKHR& surface, const uint32_t imageCount): device_{device}
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vkn::error_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.device, surface, &surfaceCapabilities), "Failed to get surface capabilities");

	auto format = getSurfaceFormat(gpu, surface);
	swapchainInfo_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo_.pNext = nullptr;
	swapchainInfo_.flags = 0;
	swapchainInfo_.surface = surface;
	if (imageCount <= 0)
	{
		swapchainInfo_.minImageCount = surfaceCapabilities.minImageCount;
	}
	else if (imageCount > surfaceCapabilities.maxImageCount)
	{
		swapchainInfo_.minImageCount = surfaceCapabilities.maxImageCount;
	}
	else
	{
		swapchainInfo_.minImageCount = imageCount;
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
	vkn::error_check(vkCreateSwapchainKHR(device_.device, &swapchainInfo_, nullptr, &swapchain_), "Unable to create the swapchain");

	retrieveImages();
}

vkn::Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(device_.device, swapchain_, nullptr);
}

void vkn::Swapchain::resize(vkn::CommandBuffer& cb, const VkExtent2D& extent)
{
	swapchainInfo_.imageExtent = extent;
	swapchainInfo_.oldSwapchain = swapchain_;
	vkn::error_check(vkCreateSwapchainKHR(device_.device, &swapchainInfo_, nullptr, &swapchain_), "Unable to REcreate the swapchain");
	retrieveImages();
	for (auto& image : images_)
	{
		image.transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
}

const VkExtent2D vkn::Swapchain::extent() const
{
	return swapchainInfo_.imageExtent;
}

void vkn::Swapchain::setImageAvailableSignal(vkn::Signal& signal, const uint64_t timeout)
{
	signal.reset();
	vkn::error_check(vkAcquireNextImageKHR(device_.device, swapchain_, timeout, signal.semaphore, signal.fence, &availableImageIndex_), "Failed to present images");
}

const VkPresentInfoKHR vkn::Swapchain::imagePresentInfo(vkn::Signal& waitOn) const
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitOn.semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain_;
	presentInfo.pImageIndices = &availableImageIndex_;
	presentInfo.pResults = nullptr;

	return presentInfo;
}

std::vector<vkn::Image>& vkn::Swapchain::images()
{
	return images_;
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

void vkn::Swapchain::retrieveImages()
{
	uint32_t count;
	vkn::error_check(vkGetSwapchainImagesKHR(device_.device, swapchain_, &count, nullptr), "Unabled to get the swapchain images count");
	std::vector<VkImage> images(count);
	vkn::error_check(vkGetSwapchainImagesKHR(device_.device, swapchain_, &count, std::data(images)), "Unabled to get the swapchain images");

	images_.clear();
	for (auto image : images)
	{
		images_.emplace_back(device_, image, swapchainInfo_.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}
