#include "..\headers\Renderpass.h"

vkn::Renderpass::Renderpass(vkn::Device& device, const VkRenderPass& renderpass, std::vector<Renderpass::Attachment>& attachments) : 
	device_{ device }, renderpass_{ renderpass },
	attachments_ { std::move(attachments) }
{
	for (const auto& attachment : attachments_)
	{
		if (isColorAttachment(attachment))
		{
			VkClearValue clear{};
			clear.color = { 0.2f, 0.2f, 0.2f, 1.0f };
			clearValues_.push_back(clear);
		}
		else
		{
			VkClearValue clear{};
			clear.depthStencil = {1.0f, 0};
			clearValues_.push_back(clear);
		}
	}
}

bool vkn::Renderpass::isColorAttachment(const Attachment& attachment) const
{
	if (attachment.format == VK_FORMAT_D16_UNORM || attachment.format == VK_FORMAT_D32_SFLOAT ||
		attachment.format == VK_FORMAT_S8_UINT || attachment.format == VK_FORMAT_D16_UNORM_S8_UINT ||
		attachment.format == VK_FORMAT_D24_UNORM_S8_UINT || attachment.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return false;
	}
	return true;
}

const VkRenderPass vkn::Renderpass::renderpass() const
{
	return renderpass_;
}

void vkn::Renderpass::begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent)
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.framebuffer = fb;
	beginInfo.renderPass = renderpass_;
	beginInfo.renderArea = renderArea;
	beginInfo.clearValueCount = std::size(clearValues_);
	beginInfo.pClearValues = std::data(clearValues_);	

	vkCmdBeginRenderPass(cb.commandBuffer(), &beginInfo, subpassContent);
}

void vkn::Renderpass::end(vkn::CommandBuffer& cb)
{
	vkCmdEndRenderPass(cb.commandBuffer());
}

vkn::Renderpass::Renderpass(Renderpass&& other): device_{other.device_}
{
	renderpass_ = other.renderpass_;
	attachments_ = std::move(other.attachments_);
	clearValues_ = std::move(other.clearValues_);
	other.renderpass_ = VK_NULL_HANDLE;
}

vkn::Renderpass::~Renderpass()
{
	if (renderpass_ != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device_.device, renderpass_, nullptr);
	}
}

const std::vector<vkn::Renderpass::Attachment>& vkn::Renderpass::attachments() const
{
	return attachments_;
}
