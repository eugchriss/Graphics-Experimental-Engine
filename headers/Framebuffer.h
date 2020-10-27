#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "image.h"
#include "queue.h"

#include <vector>

namespace vkn
{
	class Framebuffer
	{
	public:

		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, vkn::Swapchain& swapchain);
		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, const VkExtent2D& sz);
		Framebuffer(Framebuffer&& other);
		~Framebuffer();
		const VkFramebuffer frame(const uint32_t index) const;
		VkExtent2D size;


	private:
		vkn::Device& device_;
		std::vector<VkFramebuffer> framebuffers_;
		std::vector<vkn::Image> images_;

		VkImageAspectFlags getAspectFlag(const vkn::Renderpass::Attachment& attachment);
		VkImageUsageFlags getUsageFlag(const vkn::Renderpass::Attachment& attachment);
	};
}