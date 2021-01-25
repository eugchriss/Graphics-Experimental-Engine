#include "..\headers\vulkanFrameGraph.h"
#include "..\headers\Renderpass.h"
#include "..\headers\Framebuffer.h"
#include <numeric>

void vkn::FrameGraph::setFrameCount(const uint32_t count)
{
	framebufferCount_ = count;
}

void vkn::FrameGraph::setRenderArea(const uint32_t width, const uint32_t height)
{
	renderArea_.offset.x = 0;
	renderArea_.offset.y = 0;

	renderArea_.extent.width = width;
	renderArea_.extent.height = height;
}

const vkn::Attachment vkn::FrameGraph::addColorAttachment(const VkFormat format)
{
	VkAttachmentDescription attachment{};
	attachment.flags = 0;
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments_.emplace_back(attachment);
    return std::size(attachments_) - 1;
}

const vkn::Attachment vkn::FrameGraph::addDepthAttachment(const VkFormat format)
{
	VkAttachmentDescription attachment{};
	attachment.flags = 0;
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments_.emplace_back(attachment);
	return std::size(attachments_) - 1;
}

void vkn::FrameGraph::setAttachmentColorDepthContent(const Attachment attachment, const VkAttachmentLoadOp load, const VkAttachmentStoreOp store)
{
	assert(attachment <= std::size(attachments_) && "Invalid attachment");
	attachments_[attachment].loadOp = load;
	attachments_[attachment].storeOp = store;
}

void vkn::FrameGraph::setPresentAttachment(const Attachment attachment)
{
	presentAttchment_ = std::make_optional<Attachment>(attachment);
}

vkn::Pass& vkn::FrameGraph::addPass()
{
	return passes_.emplace_back();
}

vkn::RenderTarget vkn::FrameGraph::createRenderTarget(Context& context)
{
	auto renderpass = std::make_shared<vkn::Renderpass>(createRenderpass(context));
	vkn::Framebuffer framebuffer{ context, renderpass, renderArea_, framebufferCount_};

	return RenderTarget{context, std::move(renderpass), std::move(framebuffer) };
}

vkn::RenderTarget vkn::FrameGraph::createRenderTarget(Context& context, Swapchain& swapchain)
{
	assert(presentAttchment_.has_value() && "Need to set a present attachment for presenting frames");
	attachments_[*presentAttchment_].format = swapchain.imageFormat();
	attachments_[*presentAttchment_].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	auto renderpass = std::make_shared<vkn::Renderpass>(createRenderpass(context));
	vkn::Framebuffer framebuffer{ context, renderpass, swapchain, *presentAttchment_ };
	return RenderTarget{context, std::move(renderpass), std::move(framebuffer) };
}

vkn::Renderpass vkn::FrameGraph::createRenderpass(vkn::Context& context)
{
	std::vector<VkSubpassDescription> subpasses;
	for (auto& pass : passes_)
	{
		pass.setPreservedAttachments(attachments_);
		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = std::size(pass.inputAttachments);
		subpass.pInputAttachments = std::data(pass.inputAttachments);
		subpass.colorAttachmentCount = std::size(pass.colorAttachments);
		subpass.pColorAttachments = std::data(pass.colorAttachments);
		subpass.pDepthStencilAttachment = std::data(pass.depthStencilAttachments);
		subpass.pResolveAttachments = nullptr;
		subpass.preserveAttachmentCount = std::size(pass.preservedAttachments);
		subpass.pPreserveAttachments = std::data(pass.preservedAttachments);
		subpasses.push_back(subpass);
	}
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.attachmentCount = std::size(attachments_);
	info.pAttachments = std::data(attachments_);
	info.subpassCount = std::size(subpasses);
	info.pSubpasses = std::data(subpasses);
	info.dependencyCount = std::size(dependencies_);
	info.pDependencies = std::data(dependencies_);

	VkRenderPass renderpass{ VK_NULL_HANDLE };
	vkn::error_check(vkCreateRenderPass(context.device->device, &info, nullptr, &renderpass), "Failed to create the render pass");
	return Renderpass{context, renderpass, attachments_ };
}

void vkn::Pass::addColorAttachment(const Attachment attachment)
{
	VkAttachmentReference reference{};
	reference.attachment = attachment;		 
	reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachments.emplace_back(reference);
	usedAttachments.insert(attachment);
}

void vkn::Pass::addDepthStencilAttachment(const Attachment attachment)
{
	VkAttachmentReference reference{};
	reference.attachment = attachment;
	reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthStencilAttachments.emplace_back(reference);
	usedAttachments.insert(attachment);
}

void vkn::Pass::setPreservedAttachments(const std::vector<VkAttachmentDescription>& frameGraphAttachments)
{
	preservedAttachments.resize(std::size(frameGraphAttachments));
	std::iota(std::begin(preservedAttachments), std::end(preservedAttachments), 0);
	for (const auto used : usedAttachments)
	{
		auto it = std::remove(std::begin(preservedAttachments), std::end(preservedAttachments), used);
		preservedAttachments.erase(it, std::end(preservedAttachments));
	}
}