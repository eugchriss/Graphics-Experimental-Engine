#pragma once
#include "vulkan/vulkan.hpp"
#include "device.h"
#include "Renderpass.h"
#include "Shader.h"
#include <vector>

namespace vkn
{
	class RenderpassBuilder
	{
	public:
		struct Attachment
		{
			struct Content
			{
				VkAttachmentLoadOp load;
				VkAttachmentStoreOp store;
			};

			struct Layout
			{
				VkImageLayout initial;
				VkImageLayout final;
			};
		};

		struct Subpass
		{
			struct Requirement
			{
				void addInputAttachment(const uint32_t);
				void addColorAttachments(std::vector<uint32_t>& attachments);
				void addColorAttachment(const uint32_t);
				void addDepthAttachment(const uint32_t);
				void addStencilAttachment(const uint32_t);
				void setPreservedAttachments(const std::vector<RenderpassAttachment>& renderpassAttachments);
				std::vector<VkAttachmentReference> inputAttachments{};
				std::vector<VkAttachmentReference> colorAttachments{};
				std::vector<VkAttachmentReference> depthStencilAttacments;
				std::vector<uint32_t> preservedAttachments;
			private:
				std::vector<uint32_t> usedAttachments;
			};
		};

		RenderpassBuilder() = default;
		RenderpassBuilder(RenderpassBuilder&&) = default;
		const uint32_t addAttachmentT(const vkn::RenderpassAttachment& attachment, const VkImageLayout initialLayout = { VK_IMAGE_LAYOUT_UNDEFINED }, const Attachment::Content& colorDepthOp = { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE }, const Attachment::Content& stencilOp = { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE });
		const uint32_t addSubpass(Subpass::Requirement& requiments);
		void addDependecy(const vkn::SubpassAttachmentUsage &scr, const vkn::SubpassAttachmentUsage& dst);
		vkn::Renderpass get(vkn::Device& device);
		void reset();
		
	private:
		std::vector<std::pair<std::string, VkAttachmentDescription>> attachments_;
		std::vector<RenderpassBuilder::Subpass::Requirement> subpassesRequirements_;
		std::vector<VkSubpassDependency> dependencies_;
	};
}