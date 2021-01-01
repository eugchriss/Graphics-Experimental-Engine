#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "CommandBuffer.h"
#include <vector>
namespace vkn
{
	struct RenderpassAttachment
	{
		uint32_t attachmentIndex{};
		std::string name;
		VkFormat format;
		bool isUsedForPresent{ false };
	};
	struct SubpassAttachmentUsage
	{
		uint32_t subpassIndex{};
		VkPipelineStageFlags stageFlag{};
		VkAccessFlagBits accessFlag{};
	};

	class Renderpass
	{
	public:
		Renderpass(Renderpass&&);
		~Renderpass();
		void setClearColor(const glm::vec3& color);
		std::vector<RenderpassAttachment>& attachments();
		const VkRenderPass renderpass() const;
		void begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent);
		void end(vkn::CommandBuffer& cb);
		void clearDepthAttachment(vkn::CommandBuffer& cb, const VkRect2D& area, const VkClearValue& clearValue);
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
	private:
		friend class RenderpassBuilder;
		Renderpass(vkn::Device& device, const VkRenderPass& renderpass, std::vector<RenderpassAttachment>& attachments);
		vkn::Device& device_;
		VkRenderPass renderpass_{ VK_NULL_HANDLE };
		std::vector<RenderpassAttachment> attachments_;
		std::vector<VkClearValue> clearValues_;
		std::vector<size_t> colorClearValuesIndices_;
		bool isColorAttachment(const RenderpassAttachment& attachment) const;
	};
}