#pragma once
#include <vector>

#include "commandPool.h"
#include "RenderTarget.h"
#include "Signal.h"
#include "vulkanContext.h"
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"

namespace vkn
{
	class Swapchain
	{
	public:
		Swapchain(Context& context);
		~Swapchain();
		void resize(const VkExtent2D& extent);
		const VkExtent2D extent() const;
		void swapBuffers(const uint64_t timeout = UINT64_MAX);
		const VkPresentInfoKHR imagePresentInfo(Semaphore& waitOnSemaphore) const;
		std::vector<RenderTarget>& renderTargets();
		const VkFormat imageFormat() const;
	private:
		Context& context_;
		vkn::CommandPool commandPool_;
		VkSwapchainCreateInfoKHR swapchainInfo_{};
		VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
		std::vector<RenderTarget> renderTargets_;
		uint32_t availableImageIndex_{};
		const VkSurfaceFormatKHR getSurfaceFormat(vkn::Gpu& gpu, const VkSurfaceKHR& surface) const;
		void retrieveTargets();
	};
}
