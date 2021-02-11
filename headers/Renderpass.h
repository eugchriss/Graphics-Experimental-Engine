#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "CommandBuffer.h"
#include <vector>
namespace vkn
{
	struct SubpassAttachmentUsage
	{
		uint32_t subpassIndex{};
		VkPipelineStageFlags stageFlag{};
		VkAccessFlagBits accessFlag{};
	};

	class Renderpass
	{
	public:
		struct Attachment
		{
			std::string name{ "un-named" };
			VkAttachmentDescription description;
		};
		Renderpass(Context& context, const VkRenderPass& renderpass, std::vector<Attachment>& attachments);
		Renderpass(Renderpass&&);
		~Renderpass();
		void setClearColor(const glm::vec3& color);
		const std::vector<Attachment>& attachments() const;
		const VkRenderPass renderpass() const;
		void begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent);
		void end(vkn::CommandBuffer& cb);
		void clearDepthAttachment(vkn::CommandBuffer& cb, const VkRect2D& area, const VkClearValue& clearValue);
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
	private:
		friend class RenderpassBuilder;
		vkn::Device& device_;
		VkRenderPass renderpass_{ VK_NULL_HANDLE };
		std::vector<Attachment> attachments_;
		std::vector<VkClearValue> clearValues_;
		std::vector<size_t> colorClearValuesIndices_;
		bool isColorAttachment(const VkAttachmentDescription& attachment) const;
	};
}