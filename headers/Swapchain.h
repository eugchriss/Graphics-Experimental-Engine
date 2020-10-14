#pragma once
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "Gpu.h"
#include "Device.h"
#include "Signal.h"
#include "Image.h"
#include <vector>

namespace vkn
{
	class Swapchain
	{
	public:
		Swapchain(vkn::Gpu& gpu, vkn::Device& device, const VkSurfaceKHR& surface);
		~Swapchain();
		void resize(vkn::CommandBuffer& cb, const VkExtent2D& extent);
		const VkExtent2D extent() const;
		void setImageAvailableSignal(vkn::Signal& signal, const uint64_t timeout = UINT64_MAX);
		const VkPresentInfoKHR imagePresentInfo(vkn::Signal& waitOn) const;
		const std::vector<vkn::Image>& images() const;
		const VkFormat imageFormat() const;
	private:
		vkn::Device& device_;
		VkSwapchainCreateInfoKHR swapchainInfo_{};
		VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
		std::vector<vkn::Image> images_;
		uint32_t availableImageIndex_;
		const VkSurfaceFormatKHR getSurfaceFormat(vkn::Gpu& gpu, const VkSurfaceKHR& surface) const;
		void retrieveImages();
	};
}
