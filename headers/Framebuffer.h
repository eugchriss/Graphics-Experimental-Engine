#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "ShaderEffect.h"
#include "image.h"
#include "queue.h"

#include <vector>

namespace vkn
{
	class Framebuffer
	{
	public:

		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, vkn::Swapchain& swapchain);
		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, const VkExtent2D& sz, const uint32_t frameCount = 2);
		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const uint32_t frameCount, std::vector<vkn::ShaderEffect>& shaderEffects, vkn::Swapchain& swapchain);
		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const uint32_t frameCount, std::vector<vkn::ShaderEffect>& shaderEffects, const VkExtent2D& sz);
		Framebuffer(Framebuffer&& other);
		~Framebuffer();
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		const VkFramebuffer frame(const uint32_t index) const;
		const std::vector<vkn::Pixel> frameContent(const uint32_t index);
		const std::vector<float> frameRawContent(const uint32_t index);
		VkExtent2D size;


	private:
		vkn::Gpu& gpu_;
		vkn::Device& device_;
		std::vector<VkFramebuffer> framebuffers_;
		Ptr<vkn::Renderpass> renderpass_;
		std::vector<vkn::Image> images_;

		VkImageAspectFlags getAspectFlag(const vkn::Renderpass::Attachment& attachment);
		VkImageUsageFlags getUsageFlag(const vkn::Renderpass::Attachment& attachment);
	};
}