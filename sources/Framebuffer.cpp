#include "../headers/Framebuffer.h"
#include "../headers/vulkan_utils.h"
#include <cassert>

vkn::Framebuffer::Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, const VkRect2D size, const uint32_t frameCount): context_{context}, renderpass_{renderpass}
{
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = size.extent.width;
	fbInfo.height = size.extent.height;
	fbInfo.renderPass = renderpass->renderpass();
	const auto& renderpassAttachments = renderpass->attachments();
	framebuffers_.resize(frameCount);
	for (auto& framebuffer : framebuffers_)
	{
		std::vector<VkImageView> views;

		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			images_.emplace_back(context_, attachment, VkExtent3D{ size.extent.width, size.extent.height, 1 });
			views.emplace_back(images_.back().getView(getAspectFlag(attachment)));
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &framebuffer), "Failed to create the framebuffer");
	}
}

vkn::Framebuffer::Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, Swapchain& swapchain, const uint32_t presentAttachment) :
	context_{ context },
	renderpass_{ renderpass }
{
	swapchain_ = std::make_optional<std::reference_wrapper<vkn::Swapchain>>(swapchain);
	presentAttachment_ = std::make_optional(presentAttachment);
	
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = swapchain.extent().width;
	fbInfo.height = swapchain.extent().height;
	fbInfo.renderPass = renderpass->renderpass();
	const auto& renderpassAttachments = renderpass->attachments();
	framebuffers_.resize(std::size(swapchain.images()));
	auto frameIndex = 0u;
	for (auto& framebuffer : framebuffers_)
	{
		std::vector<VkImageView> views;

		views.reserve(std::size(renderpassAttachments));
		for (auto i = 0u; i < std::size(renderpassAttachments); ++i)
		{
			if (i == presentAttachment_)
			{
				views.emplace_back(swapchain.images()[frameIndex].getView(getAspectFlag(renderpassAttachments[presentAttachment])));
			}
			else
			{
				auto& attachment = renderpassAttachments[i];
				images_.emplace_back(context_, attachment, VkExtent3D{ swapchain.extent().width, swapchain.extent().height, 1 });
				views.emplace_back(images_.back().getView(getAspectFlag(attachment)));
			}
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &framebuffer), "Failed to create the framebuffer");
		++frameIndex;
	}
}

vkn::Framebuffer::~Framebuffer()
{
	for (auto fb : framebuffers_)
	{
		vkDestroyFramebuffer(context_.device->device, fb, nullptr);
	}
}

#ifndef NDEBUG
void vkn::Framebuffer::setDebugName(const std::string& name)
{
	for (auto& image : images_)
	{
		image.setDebugName(name + "image ");
	}
}
#endif
const std::vector<vkn::Pixel> vkn::Framebuffer::frameContent(const uint32_t imageIndex)
{
	return images_[imageIndex].content(*context_.gpu);
}

const float vkn::Framebuffer::rawContentAt(const VkDeviceSize offset, const uint32_t imageIndex)
{
	return images_[imageIndex].rawContentAt(*context_.gpu, offset);
}

const std::vector<float> vkn::Framebuffer::frameRawContent(const uint32_t imageIndex)
{
	return images_[imageIndex].rawContent(*context_.gpu);
}

const size_t vkn::Framebuffer::frameCount() const
{
	return std::size(images_);
}

VkFramebuffer vkn::Framebuffer::frame(const uint32_t index) const
{
	assert(std::size(framebuffers_) > index);
	return framebuffers_[index];
}

VkImageAspectFlags vkn::Framebuffer::getAspectFlag(const VkAttachmentDescription& attachment)
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

void vkn::Framebuffer::resize(const glm::u32vec2& size)
{ 
	if (swapchain_.has_value())
	{
		swapchain_->get().resize(VkExtent2D{ size.x, size.y });
	}
	images_.clear();
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = size.x;
	fbInfo.height = size.y;
	fbInfo.renderPass = renderpass_->renderpass();
	const auto& renderpassAttachments = renderpass_->attachments();
	auto frameIndex = 0u;
	for (auto& framebuffer : framebuffers_)
	{
		std::vector<VkImageView> views;
		views.reserve(std::size(renderpassAttachments));
		for (auto i = 0u; i < std::size(renderpassAttachments); ++i)
		{
			if (presentAttachment_.has_value())
			{
				if (i == presentAttachment_)
				{
					views.emplace_back(swapchain_->get().images()[frameIndex].getView(getAspectFlag(renderpassAttachments[presentAttachment_.value()])));
				}
				else
				{
					auto& attachment = renderpassAttachments[i];
					images_.emplace_back(context_, attachment, VkExtent3D{ swapchain_->get().extent().width, swapchain_->get().extent().height, 1 });
					views.emplace_back(images_.back().getView(getAspectFlag(attachment)));
				}
			}
			else
			{
				auto& attachment = renderpassAttachments[i];
				images_.emplace_back(context_, attachment, VkExtent3D{ swapchain_->get().extent().width, swapchain_->get().extent().height, 1 });
				views.emplace_back(images_.back().getView(getAspectFlag(attachment)));
			}
			
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &framebuffer), "Failed to create the framebuffer");
		++frameIndex;
	}
}