#include "../headers/Framebuffer.h"
#include "../headers/vulkan_utils.h"
#include <cassert>

vkn::Framebuffer::Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, const VkExtent2D& dimensions, const uint32_t frameCount): context_{context}, renderpass_{renderpass}, dimensions_{dimensions}
{
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = dimensions_.width;
	fbInfo.height = dimensions_.height;
	fbInfo.renderPass = renderpass->renderpass();
	const auto& renderpassAttachments = renderpass->attachments();
	framebuffers_.resize(frameCount);
	for (auto& framebuffer : framebuffers_)
	{
		std::vector<VkImageView> views;
		auto& imagesMap = images_.emplace_back();
		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			Image image{ context_, attachment.description, VkExtent3D{ dimensions_.width, dimensions_.height, 1} };
			views.emplace_back(image.getView(getAspectFlag(attachment.description)));
			imagesMap.emplace(attachment.name, std::move(image));
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &framebuffer), "Failed to create the framebuffer");
	}
}

vkn::Framebuffer::Framebuffer(Context& context, const std::shared_ptr<Renderpass>& renderpass, Swapchain& swapchain, const uint32_t presentAttachment) :
	context_{ context },
	renderpass_{ renderpass },
	dimensions_{swapchain.extent()}
{
	swapchain_ = std::make_optional<std::reference_wrapper<vkn::Swapchain>>(swapchain);
	presentAttachment_ = std::make_optional(presentAttachment);
	
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = dimensions_.width;
	fbInfo.height = dimensions_.height;
	fbInfo.renderPass = renderpass->renderpass();
	const auto& renderpassAttachments = renderpass->attachments();
	framebuffers_.resize(std::size(swapchain.images()));
	auto frameIndex = 0u;
	for (auto& framebuffer : framebuffers_)
	{
		std::vector<VkImageView> views;
		auto& imagesMap = images_.emplace_back();
		views.reserve(std::size(renderpassAttachments));
		for (auto i = 0u; i < std::size(renderpassAttachments); ++i)
		{
			if (i == presentAttachment_)
			{
				views.emplace_back(swapchain.images()[frameIndex].getView(getAspectFlag(renderpassAttachments[presentAttachment].description)));
			}
			else
			{
				auto& attachment = renderpassAttachments[i];
				Image image{ context_, attachment.description, VkExtent3D{ dimensions_.width, dimensions_.height, 1} };
				views.emplace_back(image.getView(getAspectFlag(attachment.description)));
				imagesMap.emplace(attachment.name, std::move(image));
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
	for (auto& imagesMap : images_)
	{
		for(auto& [name, image]: imagesMap)
		{
			image.setDebugName(name );
		}
	}
}
#endif

const float vkn::Framebuffer::rawContentAt(const std::string& name, const VkDeviceSize offset, const uint32_t imageIndex)
{
	auto image = images_[imageIndex].find(name);
	return image->second.rawContentAt(*context_.gpu, offset);
}

const size_t vkn::Framebuffer::frameCount() const
{
	return std::size(framebuffers_);
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
	dimensions_.width = size.x;
	dimensions_.height = size.y;
	if (swapchain_.has_value())
	{
		swapchain_->get().resize(dimensions_);
	}
	images_.clear();
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = dimensions_.width;
	fbInfo.height = dimensions_.height;
	fbInfo.renderPass = renderpass_->renderpass();
	const auto& renderpassAttachments = renderpass_->attachments();
	auto frameIndex = 0u;
	for (auto& framebuffer : framebuffers_)
	{
		vkDestroyFramebuffer(context_.device->device, framebuffer, nullptr);
		std::vector<VkImageView> views;
		auto& imagesMap = images_.emplace_back();
		views.reserve(std::size(renderpassAttachments));
		for (auto i = 0u; i < std::size(renderpassAttachments); ++i)
		{
			if (i == presentAttachment_)
			{
				views.emplace_back(swapchain_->get().images()[frameIndex].getView(getAspectFlag(renderpassAttachments[*presentAttachment_].description)));
			}
			else
			{
				auto& attachment = renderpassAttachments[i];
				Image image{ context_, attachment.description, VkExtent3D{ dimensions_.width, dimensions_.height, 1} };
				views.emplace_back(image.getView(getAspectFlag(attachment.description)));
				imagesMap.emplace(attachment.name, std::move(image));
			}
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(context_.device->device, &fbInfo, nullptr, &framebuffer), "Failed to create the framebuffer");
		++frameIndex;
	}
}

const VkExtent2D& vkn::Framebuffer::dimensions() const
{
	return dimensions_;
}

const bool vkn::Framebuffer::isOffscreen() const
{
	return !swapchain_.has_value();
}

std::unordered_map<std::string, vkn::Image>& vkn::Framebuffer::attachments(const uint32_t imageIndex)
{
	assert(imageIndex < std::size(images_) && "out of range image index");
	return images_[imageIndex];
}
