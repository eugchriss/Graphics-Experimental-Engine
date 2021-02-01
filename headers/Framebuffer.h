#pragma once
#include <vector>
#include <string>
#include <optional>
#include <utility>
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "image.h"

namespace vkn
{
	class Framebuffer
	{
	public:
		Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, const VkExtent2D& dimensions, const uint32_t frameCount = 2);
		Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, Swapchain& swapchain, const uint32_t presentAttachment);
		Framebuffer(Framebuffer&&) = default;
		~Framebuffer();
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		const std::vector<vkn::Pixel> frameContent(const uint32_t index);
		const float rawContentAt(const VkDeviceSize offset, const uint32_t imageIndex = 0u);
		const std::vector<float> frameRawContent(const uint32_t index);
		const size_t frameCount() const;
		VkFramebuffer frame(const uint32_t index) const;
		void resize(const glm::u32vec2& size);
		const VkExtent2D& dimensions() const;
		const bool isOffscreen() const;
	private:
		vkn::Context& context_;
		VkExtent2D dimensions_;
		std::vector<VkFramebuffer> framebuffers_;
		std::vector<vkn::Image> images_;
		std::shared_ptr<Renderpass> renderpass_;
		std::optional<std::reference_wrapper<Swapchain>> swapchain_;
		std::optional<uint32_t> presentAttachment_;

		VkImageAspectFlags getAspectFlag(const VkAttachmentDescription& attachment);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<VkAttachmentDescription>& renderpassAttachments, const uint32_t frameCount);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<VkAttachmentDescription>& renderpassAttachments, const uint32_t presentAttachmentIndex, const uint32_t frameCount);
	};
}