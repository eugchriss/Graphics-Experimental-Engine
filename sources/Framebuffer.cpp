#include "../headers/Framebuffer.h"
#include "../headers/vulkan_utils.h"
#include <cassert>

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, vkn::Swapchain& swapchain) : device_{ device }, size{ swapchain.extent() }
{
	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.layers = 1;
	info.height = size.height;
	info.width = size.width;
	info.renderPass = renderpass.renderpass();

	auto& swapchainImages = swapchain.images();
	const auto& renderpassAttachments = renderpass.attachments();
	uint32_t renderpassAttachmentsSize{};
	for (const auto& swapchainImage : swapchainImages)
	{
		for (const auto& attachment : renderpassAttachments)
		{
			//the renderpass attachment index 0 is already used by the swapchain images
			if (attachment.index != 0)
			{
				++renderpassAttachmentsSize;
				images_.emplace_back(gpu, device_, getUsageFlag(attachment), attachment.format, VkExtent3D{ size.width, size.height, 1 });
			}
		}
	}

	framebuffers_.resize(std::size(swapchainImages));
	for (auto i = 0u; i < std::size(swapchainImages); ++i)
	{
		std::vector<VkImageView> attachments;
		attachments.push_back(swapchainImages[i].getView(VK_IMAGE_ASPECT_COLOR_BIT));
		for (auto j = 0u; j < renderpassAttachmentsSize / std::size(swapchainImages); ++j)
		{
			attachments.push_back(images_[i * std::size(renderpassAttachments) / std::size(swapchainImages) + j].getView(VK_IMAGE_ASPECT_DEPTH_BIT));
		}
		info.attachmentCount = std::size(attachments);
		info.pAttachments = std::data(attachments);
		vkn::error_check(vkCreateFramebuffer(device_.device, &info, nullptr, &framebuffers_[i]), "Failed to create the framebuffer");
	}
}

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, const vkn::Renderpass& renderpass, const VkExtent2D& sz) : device_{ device }, size{ sz }
{
	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.layers = 1;
	info.height = size.height;
	info.width = size.width;
	info.renderPass = renderpass.renderpass();

	const auto& renderpassAttachments = renderpass.attachments();
	std::vector<VkImageView> attachmentViews;
	for (const auto& attachment : renderpassAttachments)
	{
		images_.emplace_back(gpu, device_, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, attachment.format, VkExtent3D{ size.width, size.height, 1 });
		attachmentViews.push_back(images_.back().getView(getAspectFlag(attachment)));
	}

	info.attachmentCount = std::size(attachmentViews);
	info.pAttachments = std::data(attachmentViews);
	for (const auto& image : images_)
	{
		VkFramebuffer fb{ VK_NULL_HANDLE };
		vkn::error_check(vkCreateFramebuffer(device_.device, &info, nullptr, &fb), "Failed to create the framebuffer");
		framebuffers_.emplace_back(fb);
	}

}

vkn::Framebuffer::Framebuffer(Framebuffer&& other) : device_{ other.device_ }
{
	framebuffers_ = std::move(other.framebuffers_);
	images_ = std::move(other.images_);
	size = other.size;
}

vkn::Framebuffer::~Framebuffer()
{
	for (auto fb : framebuffers_)
	{
		vkDestroyFramebuffer(device_.device, fb, nullptr);
	}
}

const VkFramebuffer vkn::Framebuffer::frame(const uint32_t index) const
{
	assert(index <= std::size(framebuffers_) && "There is no image at that index");
	return framebuffers_[index];
}

VkImageAspectFlags vkn::Framebuffer::getAspectFlag(const vkn::Renderpass::Attachment& attachment)
{
	if (attachment.format == VK_FORMAT_D16_UNORM || attachment.format == VK_FORMAT_D32_SFLOAT)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (attachment.format == VK_FORMAT_S8_UINT)
	{
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if (attachment.format == VK_FORMAT_D16_UNORM_S8_UINT || attachment.format == VK_FORMAT_D24_UNORM_S8_UINT || attachment.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

VkImageUsageFlags vkn::Framebuffer::getUsageFlag(const vkn::Renderpass::Attachment& attachment)
{
	if (attachment.format == VK_FORMAT_D16_UNORM || attachment.format == VK_FORMAT_D32_SFLOAT ||
		attachment.format == VK_FORMAT_S8_UINT || attachment.format == VK_FORMAT_D16_UNORM_S8_UINT ||
		attachment.format == VK_FORMAT_D24_UNORM_S8_UINT || attachment.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
}
