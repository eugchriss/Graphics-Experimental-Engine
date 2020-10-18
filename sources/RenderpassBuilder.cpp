#include "../headers/renderpassBuilder.h"
#include "../headers/vulkan_utils.h"

vkn::RenderpassBuilder::RenderpassBuilder(vkn::Device& device) : device_{ device }
{
}

const std::vector<uint32_t> vkn::RenderpassBuilder::addAttachments(const std::vector<VkFormat>& formats, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp, const Attachment::Layout& layout)
{
	std::vector<uint32_t> attachments;
	attachments.reserve(std::size(formats));

	for (const auto format : formats)
	{
		VkAttachmentDescription attachment{};
		attachment.flags = 0;
		attachment.format = format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = colorDepthOp.load;
		attachment.storeOp = colorDepthOp.store;
		attachment.stencilLoadOp = stencilOp.load;
		attachment.stencilStoreOp = stencilOp.store;
		attachment.initialLayout = layout.initial;
		attachment.finalLayout = layout.final;

		attachments_.push_back(attachment);
	}
	attachments.push_back(std::size(attachments_) - 1);
	return attachments;
}

const uint32_t vkn::RenderpassBuilder::addAttachment(const VkFormat format, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp, const Attachment::Layout& layout)
{
	VkAttachmentDescription attachment{};
	attachment.flags = 0;
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = colorDepthOp.load;
	attachment.storeOp = colorDepthOp.store;
	attachment.stencilLoadOp = stencilOp.load;
	attachment.stencilStoreOp = stencilOp.store;
	attachment.initialLayout = layout.initial;
	attachment.finalLayout = layout.final;

	attachments_.push_back(attachment);
	return std::size(attachments_) - 1;
}

const uint32_t vkn::RenderpassBuilder::addSubpass(Subpass::Requirement& requiments)
{
	VkSubpassDescription subpass{};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = std::size(requiments.inputAttachments);
	subpass.pInputAttachments = std::data(requiments.inputAttachments);
	subpass.colorAttachmentCount = std::size(requiments.colorAttachments);
	subpass.pColorAttachments = std::data(requiments.colorAttachments);
	subpass.pDepthStencilAttachment = std::data(requiments.depthStencilAttacments);
	subpass.pResolveAttachments = nullptr;
	subpass.preserveAttachmentCount = 0;

	subpasses_.push_back(subpass);
	return std::size(subpasses_) - 1;
}

void vkn::RenderpassBuilder::addDependecy(const Dependency::Subpass& src, const Dependency::Subpass& dst)
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = src.subpass;
	dependency.srcStageMask = src.stageFlag;
	dependency.srcAccessMask = src.accesFlag;
	dependency.dstSubpass = dst.subpass;
	dependency.dstStageMask = dst.stageFlag;
	dependency.dstAccessMask = dst.accesFlag;
	dependency.dependencyFlags = 0;
	dependencies_.push_back(dependency);
}

vkn::Renderpass vkn::RenderpassBuilder::get()
{
	VkRenderPass renderpass{ VK_NULL_HANDLE };
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.attachmentCount = std::size(attachments_);
	info.pAttachments = std::data(attachments_);
	info.subpassCount = std::size(subpasses_);
	info.pSubpasses = std::data(subpasses_);
	info.dependencyCount = std::size(dependencies_);
	info.pDependencies = std::data(dependencies_);

	vkn::error_check(vkCreateRenderPass(device_.device, &info, nullptr, &renderpass), "Failed to create the render pass");

	std::vector<vkn::Renderpass::Attachment> attachments;
	for (auto i = 0u; i < std::size(attachments_); ++i)
	{
		vkn::Renderpass::Attachment attachment;
		attachment.index = i;
		attachment.format = attachments_[i].format;
		attachments.push_back(attachment);
	}
	return vkn::Renderpass{ device_, renderpass, attachments };
}

void vkn::RenderpassBuilder::reset()
{
	attachments_.resize(0);
	subpasses_.resize(0);
	dependencies_.resize(0);
}

void vkn::RenderpassBuilder::Subpass::Requirement::addInputAttachment(const uint32_t attachment, const VkImageLayout layout)
{
	inputAttachments.push_back(VkAttachmentReference{ attachment, layout });
}

void vkn::RenderpassBuilder::Subpass::Requirement::addColorAttachments(std::vector<uint32_t>& attachments)
{
	for (const auto attachment : attachments)
	{
		colorAttachments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	}
}

void vkn::RenderpassBuilder::Subpass::Requirement::addColorAttachment(const uint32_t attachment)
{
	colorAttachments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
}

void vkn::RenderpassBuilder::Subpass::Requirement::addDepthStencilAttachment(const uint32_t attachment)
{
	depthStencilAttacments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
}
