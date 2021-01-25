#pragma once
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "vulkanContext.h"
#include "Signal.h"
#include "Image.h"
#include <vector>

namespace vkn
{
	class Swapchain
	{
	public:
		Swapchain(Context& context);
		~Swapchain();
		void resize(const VkExtent2D& extent);
		const VkExtent2D extent() const;
		void setImageAvailableSignal(vkn::Signal& signal, const uint64_t timeout = UINT64_MAX);
		const VkPresentInfoKHR imagePresentInfo(vkn::Signal& waitOn) const;
		std::vector<vkn::Image>& images();
		const VkFormat imageFormat() const;
	private:
		Context& context_;
		VkSwapchainCreateInfoKHR swapchainInfo_{};
		VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
		std::vector<vkn::Image> images_;
		uint32_t availableImageIndex_;
		const VkSurfaceFormatKHR getSurfaceFormat(vkn::Gpu& gpu, const VkSurfaceKHR& surface) const;
		void retrieveImages();
	};
}
