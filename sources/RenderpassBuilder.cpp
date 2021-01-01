#include "../headers/renderpassBuilder.h"
#include "../headers/vulkan_utils.h"
#include <numeric>

const uint32_t vkn::RenderpassBuilder::addAttachmentT(const vkn::RenderpassAttachment& attachment, const VkImageLayout finalLayout, const VkImageLayout initialLayout, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp)
{
	auto result = std::find_if(std::begin(attachments_), std::end(attachments_), [&](const auto pair) { return pair.first == attachment.name; });
	if (result == std::end(attachments_))
	{
		VkAttachmentDescription att{};
		att.flags = 0;
		att.format = attachment.format;
		att.samples = VK_SAMPLE_COUNT_1_BIT;
		att.loadOp = colorDepthOp.load;
		att.storeOp = colorDepthOp.store;
		att.stencilLoadOp = stencilOp.load;
		att.stencilStoreOp = stencilOp.store;
		att.initialLayout = initialLayout;
		att.finalLayout = finalLayout;
		attachments_.emplace_back(attachment.name, att);
		return std::size(attachments_) - 1;
	}
	else
	{
		return std::distance(std::begin(attachments_), result);
	}
}

const uint32_t vkn::RenderpassBuilder::addSubpass(Subpass::Requirement& requiment)
{
	subpassesRequirements_.push_back(requiment);
	return std::size(subpassesRequirements_) - 1;
}

void vkn::RenderpassBuilder::addDependecy(const vkn::SubpassAttachmentUsage& src, const vkn::SubpassAttachmentUsage& dst)
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = src.subpassIndex;
	dependency.srcStageMask = src.stageFlag;
	dependency.srcAccessMask = src.accessFlag;
	dependency.dstSubpass = dst.subpassIndex;
	dependency.dstStageMask = dst.stageFlag;
	dependency.dstAccessMask = dst.accessFlag;
	dependency.dependencyFlags = 0;
	dependencies_.push_back(dependency);
}

vkn::Renderpass vkn::RenderpassBuilder::get(vkn::Device& device)
{
	std::vector<VkSubpassDescription> subpasses_;
	for (const auto& requirement : subpassesRequirements_)
	{
		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = std::size(requirement.inputAttachments);
		subpass.pInputAttachments = std::data(requirement.inputAttachments);
		subpass.colorAttachmentCount = std::size(requirement.colorAttachments);
		subpass.pColorAttachments = std::data(requirement.colorAttachments);
		subpass.pDepthStencilAttachment = std::data(requirement.depthStencilAttacments);
		subpass.pResolveAttachments = nullptr;
		subpass.preserveAttachmentCount = std::size(requirement.preservedAttachments);
		subpass.pPreserveAttachments = std::data(requirement.preservedAttachments);
		subpasses_.push_back(subpass);
	}
	std::vector<VkAttachmentDescription> attachmentsDesc;
	for (const auto attachment : attachments_)
	{
		attachmentsDesc.push_back(attachment.second);
	}
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.attachmentCount = std::size(attachments_);
	info.pAttachments = std::data(attachmentsDesc);
	info.subpassCount = std::size(subpasses_);
	info.pSubpasses = std::data(subpasses_);
	info.dependencyCount = std::size(dependencies_);
	info.pDependencies = std::data(dependencies_);

	VkRenderPass renderpass{ VK_NULL_HANDLE };
	vkn::error_check(vkCreateRenderPass(device.device, &info, nullptr, &renderpass), "Failed to create the render pass");

	std::vector<vkn::RenderpassAttachment> attachments;
	for (auto i = 0u; i < std::size(attachments_); ++i)
	{
		vkn::RenderpassAttachment attachment{};
		attachment.attachmentIndex = i;
		attachment.format = attachments_[i].second.format;
		attachment.name = attachments_[i].first;
		attachments.emplace_back(attachment);
	}
	return vkn::Renderpass{ device, renderpass, attachments};
}

void vkn::RenderpassBuilder::reset()
{
	attachments_.clear();
	subpassesRequirements_.clear();
	dependencies_.clear();
}

void vkn::RenderpassBuilder::Subpass::Requirement::addInputAttachment(const uint32_t attachment)
{
	inputAttachments.push_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	usedAttachments.push_back(attachment);
}

void vkn::RenderpassBuilder::Subpass::Requirement::addColorAttachments(std::vector<uint32_t>& attachments)
{
	for (const auto attachment : attachments)
	{
		colorAttachments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		usedAttachments.push_back(attachment);
	}
}

void vkn::RenderpassBuilder::Subpass::Requirement::addColorAttachment(const uint32_t attachment)
{
	colorAttachments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	usedAttachments.push_back(attachment);
}

void vkn::RenderpassBuilder::Subpass::Requirement::addDepthAttachment(const uint32_t attachment)
{
	depthStencilAttacments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	usedAttachments.push_back(attachment);
}

void vkn::RenderpassBuilder::Subpass::Requirement::addStencilAttachment(const uint32_t attachment)
{
	depthStencilAttacments.emplace_back(VkAttachmentReference{ attachment, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL });
	usedAttachments.push_back(attachment);
}

void vkn::RenderpassBuilder::Subpass::Requirement::setPreservedAttachments(const std::vector<RenderpassAttachment>& renderpassAttachments)
{
	preservedAttachments.resize(std::size(renderpassAttachments));
	std::iota(std::begin(preservedAttachments), std::end(preservedAttachments), 0);
	for (const auto used : usedAttachments)
	{
		preservedAttachments.erase(std::remove(std::begin(preservedAttachments), std::end(preservedAttachments), used));
	}
}
