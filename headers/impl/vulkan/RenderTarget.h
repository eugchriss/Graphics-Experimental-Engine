#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "CommandBuffer.h"
#include "Image.h"
#include "vulkanContext.h"
#include "glm/glm.hpp"

namespace vkn
{
	class RenderTarget
	{
	public:

		RenderTarget(Context& context, const VkFormat format, const VkExtent2D& size, const VkImageUsageFlags usage, const VkImageLayout finalLayout);
		//This ctor is specific for the swapchain's images
		RenderTarget(Context& context, VkImage image, const VkFormat format);
		RenderTarget(RenderTarget&&) = default;
		void resize(const VkExtent2D& newExtent);
		void clear(CommandBuffer& cb);
		VkFormat format{ VK_FORMAT_UNDEFINED };
		VkImageView view(const VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t layerCount = 1);
		const VkClearValue get_clear_value() const;
		void transition_layout(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const VkImageLayout newLayout);
		glm::vec3 clearColor{};
		glm::vec2 clearDepthStencil{1.0f, 0};
		VkImageLayout finalLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkAttachmentLoadOp loadOperation{ VK_ATTACHMENT_LOAD_OP_DONT_CARE };
		VkAttachmentStoreOp storeOperation{ VK_ATTACHMENT_STORE_OP_DONT_CARE };
		VkAttachmentLoadOp stencilLoadOperation{ VK_ATTACHMENT_LOAD_OP_DONT_CARE };
		VkAttachmentStoreOp stencilStoreOperation{ VK_ATTACHMENT_STORE_OP_DONT_CARE };
		const uint64_t id() const;

	private:
		Context& context_;
		std::unique_ptr<Image> image_;
		VkImageUsageFlags usage_{};
		VkExtent2D extent_{};
	};

	MAKE_REFERENCE(RenderTarget);
}
