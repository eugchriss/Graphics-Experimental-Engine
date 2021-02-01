#pragma once
#include <vector>
#include <optional>
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Swapchain.h"
#include "RenderTarget.h"

namespace vkn
{
	using Attachment = uint32_t;

	class Pass
	{
	public:
		void addColorAttachment(const Attachment attachment);
		void addDepthStencilAttachment(const Attachment attachment);
		const uint32_t index() const;
		friend class FrameGraph;
	private:
		Pass(const uint32_t index);
		void setPreservedAttachments(const std::vector<VkAttachmentDescription>& frameGraphAttachments);
		std::vector<VkAttachmentReference> inputAttachments{};
		std::vector<VkAttachmentReference> colorAttachments{};
		std::vector<VkAttachmentReference> depthStencilAttachments;
		std::vector<uint32_t> preservedAttachments;
		std::unordered_set<Attachment> usedAttachments;
		uint32_t index_;
		
	};
	class FrameGraph
	{
	public:
		FrameGraph() = default;
		void setRenderArea(const uint32_t width, const uint32_t height);
		const Attachment addColorAttachment(const VkFormat format, const VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		const Attachment addDepthAttachment(const VkFormat format, const VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		void setAttachmentColorDepthContent(const Attachment attchment, const VkAttachmentLoadOp load, const VkAttachmentStoreOp store);
		void setPresentAttachment(const Attachment attachment);
		Pass& addPass();
		RenderTarget createRenderTarget(Context& context, const uint32_t frameCount = 1);
		RenderTarget createRenderTarget(Context& context, Swapchain& swapchain);
	private:
		std::vector<VkAttachmentDescription> attachments_;
		std::vector<Pass> passes_;
		std::vector<VkSubpassDependency> dependencies_;
		std::optional<Attachment> presentAttchment_;
		VkExtent2D renderArea_{};
		Renderpass createRenderpass(Context& context);
		void findDependencies();
	};
}