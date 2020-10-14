#pragma once
#include "vulkan/vulkan.hpp"
#include "device.h"
#include "Renderpass.h"
#include <vector>

namespace vkn
{
	class RenderpassBuilder
	{
	public:
		struct Attachment
		{
			struct Content
			{
				VkAttachmentLoadOp load;
				VkAttachmentStoreOp store;
			};

			struct Layout
			{
				VkImageLayout initial;
				VkImageLayout final;
			};
		};
		struct Subpass
		{
			struct Requirement
			{
				void addInputAttachment(const uint32_t, const VkImageLayout layout);
				void addColorAttachment(const uint32_t, const VkImageLayout layout);
				void addDepthStencilAttachment(const uint32_t, const VkImageLayout layout);
				std::vector<VkAttachmentReference> inputAttachments{};
				std::vector<VkAttachmentReference> colorAttachments{};
				std::vector<VkAttachmentReference> depthStencilAttacments;
			};
		};
		struct Dependency
		{
			struct Subpass
			{
				uint32_t subpass;
				VkPipelineStageFlags stageFlag;
				VkAccessFlags accesFlag;
			};
		};

		RenderpassBuilder(vkn::Device& device);
		const uint32_t addAttachment(const VkFormat format, const Attachment::Content& colorDepthOp, const Attachment::Content& stencilOp, const Attachment::Layout& layout);
		const uint32_t addSubpass(Subpass::Requirement& requiments);
		void addDependecy(const Dependency::Subpass& scr, const Dependency::Subpass& dst);
		vkn::Renderpass get();
		void reset();

	private:
		vkn::Device& device_;
		std::vector<VkAttachmentDescription> attachments_;
		std::vector<VkSubpassDescription> subpasses_;
		std::vector<VkSubpassDependency> dependencies_;
	};
}