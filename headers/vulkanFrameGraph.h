#pragma once
#include <vector>
#include <optional>
#include <unordered_set>
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Swapchain.h"
#include "RenderTarget.h"

namespace vkn
{
	class RenderTarget;
	using Attachment = uint32_t;

	class Pass
	{
	public:
		void addColorAttachment(const Attachment attachment);
		void addDepthStencilAttachment(const Attachment attachment);
	private:
		friend class FrameGraph;
		void setPreservedAttachments(const std::vector<VkAttachmentDescription>& frameGraphAttachments);
		std::vector<VkAttachmentReference> inputAttachments{};
		std::vector<VkAttachmentReference> colorAttachments{};
		std::vector<VkAttachmentReference> depthStencilAttachments;
		std::vector<uint32_t> preservedAttachments;
		std::unordered_set<Attachment> usedAttachments;
		
	};
	class FrameGraph
	{
	public:
		FrameGraph() = default;
		void setFrameCount(const uint32_t count);
		void setRenderArea(const uint32_t width, const uint32_t height);
		const Attachment addColorAttachment(const VkFormat format);
		const Attachment addDepthAttachment(const VkFormat format);
		void setAttachmentColorDepthContent(const Attachment attchment, const VkAttachmentLoadOp load, const VkAttachmentStoreOp store);
		void setPresentAttachment(const Attachment attachment);
		Pass& addPass();
		RenderTarget createRenderTarget(Context& context);
		RenderTarget createRenderTarget(Context& context, Swapchain& swapchain);
	private:
		std::vector<VkAttachmentDescription> attachments_;
		std::vector<Pass> passes_;
		std::vector<VkSubpassDependency> dependencies_;
		std::optional<Attachment> presentAttchment_;
		uint32_t framebufferCount_{ 2 };
		VkRect2D renderArea_{};
		Renderpass createRenderpass(Context& context);
	};
}