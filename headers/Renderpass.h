#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "CommandBuffer.h"
#include <vector>

namespace vkn
{
	class Renderpass
	{
	public:
		struct Attachment
		{
			uint32_t index;
			VkFormat format;
		};
		Renderpass(Renderpass&&);
		~Renderpass();
		const std::vector<Attachment>& attachments() const;
		const VkRenderPass renderpass() const;
		void begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent);
		void end(vkn::CommandBuffer& cb);
	private:
		friend class RenderpassBuilder;
		Renderpass(vkn::Device& device, const VkRenderPass& renderpass, std::vector<Renderpass::Attachment>& attachments);
		vkn::Device& device_;
		VkRenderPass renderpass_{ VK_NULL_HANDLE };
		std::vector<Attachment> attachments_;
		std::vector<VkClearValue> clearValues_;

		bool isColorAttachment(const Attachment& attachment) const;
	};
}