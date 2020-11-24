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
				void addInputAttachment(const uint32_t, const VkImageLayout layout);
				void addColorAttachments(std::vector<uint32_t>& attachments);
				void addColorAttachment(const uint32_t);
				void addDepthStencilAttachment(const uint32_t);
				std::vector<VkAttachmentReference> inputAttachments{};
				std::vector<VkAttachmentReference> colorAttachments{};
				std::vector<VkAttachmentReference> depthStencilAttacments;
			};
		};
		struct Dependency
		{
			struct Subpass
			{
				uint32_t subpass;
				VkPipelineStageFlags stageFlag;
				VkAccessFlags accesFlag;
			};
		};

		RenderpassBuilder(vkn::Device& device);
		RenderpassBuilder(RenderpassBuilder&&) = default;
		const uint32_t addAttachment(const VkFormat format, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp, const Attachment::Layout& layout);
		const uint32_t addAttachment(const vkn::Shader::Attachment& attachment, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp, const Attachment::Layout& layout);
		const uint32_t addSubpass(Subpass::Requirement& requiments);
		void addDependecy(const Dependency::Subpass& scr, const Dependency::Subpass& dst);
		vkn::Renderpass get();
		void reset();
		static RenderpassBuilder getDefaultColorDepthResolveRenderpass(vkn::Device& device, const VkFormat attachmentFormat, const VkAttachmentLoadOp loadOp, const VkImageLayout initialLayout, const VkImageLayout finalLayout);

	private:
		vkn::Device& device_;
		std::vector<std::pair<std::string, VkAttachmentDescription>> attachments_;
		std::vector<RenderpassBuilder::Subpass::Requirement> subpassesRequirements_;
		std::vector<VkSubpassDependency> dependencies_;
	};
}