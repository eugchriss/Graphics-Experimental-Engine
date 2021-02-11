#include "..\headers\vulkanFrameGraph.h"
#include "..\headers\Renderpass.h"
#include "..\headers\Framebuffer.h"
#include <numeric>
#include <unordered_map>

void vkn::FrameGraph::setRenderArea(const uint32_t width, const uint32_t height)
{
	renderArea_.width = width;
	renderArea_.height = height;
}

const vkn::Attachment vkn::FrameGraph::addColorAttachment(const std::string& name, const VkFormat format, const VkImageLayout layout)
{
	Attachment att;
	att.name = name;
	att.index = std::size(attachments_);

	VkAttachmentDescription attachment{};
	attachment.flags = 0;
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = layout;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments_.emplace_back(std::make_pair(att, attachment));
	return att;
}

const vkn::Attachment vkn::FrameGraph::addDepthAttachment(const std::string& name, const VkFormat format, const VkImageLayout layout)
{
	Attachment att;
	att.name = name;
	att.index = std::size(attachments_);

	VkAttachmentDescription attachment{};
	attachment.flags = 0;
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = layout;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments_.emplace_back(std::make_pair(att, attachment));
	return att;
}

void vkn::FrameGraph::setAttachmentColorDepthContent(const Attachment attachment, const VkAttachmentLoadOp load, const VkAttachmentStoreOp store)
{
	assert(attachment.index <= std::size(attachments_) && "Invalid attachment");
	attachments_[attachment.index].second.loadOp = load;
	attachments_[attachment.index].second.storeOp = store;
}

void vkn::FrameGraph::setPresentAttachment(const Attachment attachment)
{
	presentAttchment_ = std::make_optional<Attachment>(attachment);
}

vkn::Pass& vkn::FrameGraph::addPass()
{
	return passes_.emplace_back(Pass{ static_cast<uint32_t>(std::size(passes_)) });
}

vkn::RenderTarget vkn::FrameGraph::createRenderTarget(Context& context, const uint32_t frameCount)
{
	auto renderpass = std::make_shared<vkn::Renderpass>(createRenderpass(context));
	vkn::Framebuffer framebuffer{ context, renderpass, renderArea_, frameCount};

	return RenderTarget{context, std::move(renderpass), std::move(framebuffer) };
}

vkn::RenderTarget vkn::FrameGraph::createRenderTarget(Context& context, Swapchain& swapchain)
{
	assert(presentAttchment_.has_value() && "Need to set a present attachment for presenting frames");
	attachments_[presentAttchment_->index].second.format = swapchain.imageFormat();
	attachments_[presentAttchment_->index].second.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	auto renderpass = std::make_shared<vkn::Renderpass>(createRenderpass(context));

	vkn::Framebuffer framebuffer{ context, renderpass, swapchain, presentAttchment_->index };
	return RenderTarget{context, std::move(renderpass), std::move(framebuffer) };
}

vkn::Renderpass vkn::FrameGraph::createRenderpass(vkn::Context& context)
{
	std::vector<VkAttachmentDescription> descriptions;
	descriptions.reserve(std::size(attachments_));
	for (const auto& [attachment, description] : attachments_)
	{
		descriptions.emplace_back(description);
	}

	std::vector<VkSubpassDescription> subpasses;
	for (auto& pass : passes_)
	{
		pass.setPreservedAttachments(descriptions);
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
	findDependencies();
	VkRenderPassCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.attachmentCount = std::size(descriptions);
	info.pAttachments = std::data(descriptions);
	info.subpassCount = std::size(subpasses);
	info.pSubpasses = std::data(subpasses);
	info.dependencyCount = std::size(dependencies_);
	info.pDependencies = std::data(dependencies_);

	VkRenderPass renderpass{ VK_NULL_HANDLE };
	vkn::error_check(vkCreateRenderPass(context.device->device, &info, nullptr, &renderpass), "Failed to create the render pass");
	std::vector<Renderpass::Attachment> attachments;
	attachments.reserve(std::size(attachments_));
	for (const auto& [attachment, description] : attachments_)
	{
		Renderpass::Attachment att;
		att.name = attachment.name;
		att.description = description;
		attachments.emplace_back(att);
	}
	return Renderpass{context, renderpass, attachments};
}

void vkn::FrameGraph::findDependencies()
{
	std::unordered_map<uint32_t, std::vector<SubpassAttachmentUsage>> attachmentUsages;
	for (auto i = 0u; i < std::size(passes_); ++i)
	{
		const auto& pass = passes_[i];
		vkn::SubpassAttachmentUsage usage{}; 
		usage.subpassIndex = i;
		for (const auto& attachment : pass.inputAttachments)
		{
			usage.stageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //Assuming that for now all input attachments are consumed in the frag shader
			usage.accessFlag = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			attachmentUsages[attachment.attachment].push_back(usage);
		}
		for (const auto& attachment : pass.colorAttachments)
		{
			usage.stageFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			usage.accessFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			attachmentUsages[attachment.attachment].push_back(usage);
		}
		for (const auto& attachment : pass.depthStencilAttachments)
		{
			usage.stageFlag = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			usage.accessFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			attachmentUsages[attachment.attachment].push_back(usage);
		}
	}

	for (const auto& [attachment, usages] : attachmentUsages)
	{
		if (std::size(usages) > 1)
		{
			for (auto i = 0u; i < std::size(usages) - 1; ++i)
			{
				auto& src = usages[i];
				auto& dst = usages[i+1];

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
		}
	}
}

void vkn::Pass::addColorAttachment(const Attachment attachment)
{
	VkAttachmentReference reference{};
	reference.attachment = attachment.index;		 
	reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachments.emplace_back(reference);
	usedAttachments.insert(attachment.index);
}

void vkn::Pass::addDepthStencilAttachment(const Attachment attachment)
{
	VkAttachmentReference reference{};
	reference.attachment = attachment.index;
	reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthStencilAttachments.emplace_back(reference);
	usedAttachments.insert(attachment.index);
}

void vkn::Pass::addInputAttachment(const Attachment attachment)
{
	VkAttachmentReference reference{};
	reference.attachment = attachment.index;
	reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputAttachments.emplace_back(reference);
	usedAttachments.insert(attachment.index);
}

const uint32_t vkn::Pass::index() const
{
	return index_;
}

vkn::Pass::Pass(const uint32_t index): index_{index}
{
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