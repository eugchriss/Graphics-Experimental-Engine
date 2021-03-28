#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
#include "Pass.h"
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
		Renderpass(Context& context, const VkExtent2D& extent, std::vector<Pass>&& passes, const uint32_t framebufferCount = 2);
		Renderpass(Renderpass&&);
		~Renderpass();
		const VkRenderPass& operator()() const;
		const uint32_t passesCount() const;
		void resize(const glm::u32vec2& newSize);
		void begin(vkn::CommandBuffer& cb, const VkRect2D& renderArea, const VkSubpassContents subpassContent = VK_SUBPASS_CONTENTS_INLINE);
		void end(vkn::CommandBuffer& cb);
		void draw(vkn::CommandBuffer& cb, const uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,	uint32_t firstInstance);
	private:
		Context& context_;
		VkRenderPass renderpass_{ VK_NULL_HANDLE };
		uint32_t currentFramebuffer{};
		std::vector<VkFramebuffer> framebuffers_;
		std::vector<Pass> passes_;
		std::vector<VkClearValue> clearValues_;
		std::vector<RenderTargetRef> getUniqueRenderTargets();
		void createRenderpass();
		struct SubpassDatas
		{
			std::vector<VkAttachmentReference> inputAttachments;
			std::vector<VkAttachmentReference> colorAttachments;
			std::vector<VkAttachmentReference> depthStencilAttachments;
			std::vector<uint32_t> preservedAttachments;
		};
		const std::vector<VkAttachmentDescription> getAttachments(const std::vector<RenderTargetRef>& targets) const;
		const std::vector<SubpassDatas> getSubpassesDatas(const std::vector<RenderTargetRef>& targets);
		const std::vector<VkSubpassDependency> findDependencies(const std::vector<RenderTargetRef>& targets);
		void createFramebuffers(const uint32_t framebufferCount, const VkExtent2D& extent);
	};
	MAKE_REFERENCE(Renderpass);
}