#include "../../headers/impl/vulkan/RenderTarget.h"

using namespace gee;
vkn::RenderTarget::RenderTarget(Context& context, const VkFormat fmt, const VkExtent2D& extent, const VkImageUsageFlags usage, const VkImageLayout finalLayout):
	context_{context}, format{fmt}, extent_{extent},
	usage_{usage}, finalLayout{finalLayout}
{
	
	assert(((usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) 
		 ^ ((usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		&& "Rendertarget usage must be VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT xor VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT");
	image_ = std::make_unique<Image>(context_, usage_, format, VkExtent3D{ extent.width, extent.height, 1 });
}

vkn::RenderTarget::RenderTarget(Context& context, VkImage image, const VkFormat fmt):
	context_{ context }, format{ fmt },
	usage_{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT }, finalLayout{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
{
	image_ = std::make_unique<Image>(context_, image, format);
}

void vkn::RenderTarget::resize(const VkExtent2D& newExtent)
{
	extent_ = newExtent;
	image_ = std::make_unique<Image>(context_, usage_, format, VkExtent3D{ extent_.width, extent_.height, 1 });
}

void vkn::RenderTarget::clear(CommandBuffer& cb)
{
	VkClearAttachment clearAttachment{};
	if (usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		throw std::runtime_error{ "color clear is not implemented yet" };
	}
	else if (usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		VkClearValue clearValue{};
		clearValue.depthStencil = { clearDepthStencil.r, static_cast<uint32_t>(clearDepthStencil.g)};
		clearAttachment.clearValue = clearValue;
	}
	else
	{
		//this should never happen due to the assert in ctor, but just in case...
		throw std::runtime_error{ "wrong rendertarget usage" };
	}

	VkRect2D area{};
	area.offset = { 0,0 };
	area.extent = extent_;

	VkClearRect clearRect{};
	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect = area;
	vkCmdClearAttachments(cb.commandBuffer(), 1, &clearAttachment, 1, &clearRect);
}

VkImageView vkn::RenderTarget::view(const VkImageViewType viewType, const uint32_t layerCount)
{
	VkImageAspectFlags aspect{};
	if ((usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else if ((usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	assert(aspect != 0 && "Unabled to find the image's aspect");
	return image_->getView(aspect, viewType, layerCount);
}

const VkClearValue vkn::RenderTarget::get_clear_value() const
{
	VkClearValue clearValue{};
	if ((usage_ & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		clearValue.color = { clearColor.r, clearColor.g, clearColor.b, 0.0f };
	}
	else if ((usage_ & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		clearValue.depthStencil = { clearDepthStencil.x, static_cast<uint32_t>(clearDepthStencil.y) };
	}
	return clearValue;
}

void vkn::RenderTarget::transition_layout(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const VkImageLayout newLayout)
{
	assert(image_);
	image_->transitionLayout(cb, aspect, newLayout);
}

const uint64_t vkn::RenderTarget::id() const
{
	return reinterpret_cast<uint64_t>(image_->image);
}
