#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "CommandBuffer.h"
#include "RenderpassBuilder.h"
#include "PipelineBuilder.h"
#include "Renderpass.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Framebuffer.h"
#include "Drawable.h"

#include <functional>
#include <memory>
#include <vector>
namespace vkn
{
	class ShaderTechnique
	{
	public:
		ShaderTechnique(vkn::RenderpassBuilder& renderpassBuilder, vkn::PipelineBuilder& pipelineBuilder, vkn::Swapchain& swapchain);
		ShaderTechnique(vkn::RenderpassBuilder& renderpassBuilder, vkn::PipelineBuilder& pipelineBuilder, const VkExtent2D& size);
		ShaderTechnique(ShaderTechnique&&) = default;
		
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		void setClearColor(const glm::vec3& color);
		void updatePipelineTexture(const std::string& resourceName, const VkSampler& sampler, const VkImageView& view, const VkShaderStageFlagBits stage);
		void updatePipelineTextures(const std::string& resourceName, const VkSampler& sampler, const std::vector<VkImageView>& views, const VkShaderStageFlagBits stage);
		VkFramebuffer framebuffer(const uint32_t index);
		const std::vector<vkn::Pixel> content(const uint32_t framebufferImageIndex);
		template<class T>
		void pipelinePushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas);
		template<class T>
		void updatePipelineBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage);


		template<class Fn>
		void execute(vkn::CommandBuffer& cb, const uint8_t currentFrame, const VkViewport& viewport, const VkRect2D& scissor, Fn drawCalls);
	private:
		std::unique_ptr<vkn::Pipeline> pipeline_;
		std::unique_ptr<vkn::Renderpass> renderpass_;
		std::unique_ptr<vkn::Framebuffer> framebuffer_;
		
	};
	template<class T>
	inline void ShaderTechnique::pipelinePushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas)
	{
		pipeline_->pushConstant(cb, name, datas);
	}
	template<class T>
	inline void ShaderTechnique::updatePipelineBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage)
	{
		pipeline_->updateBuffer(resourceName, datas, stage);
	}
	template<class Fn>
	inline void ShaderTechnique::execute(vkn::CommandBuffer& cb, const uint8_t currentFrame, const VkViewport& viewport, const VkRect2D& scissor, Fn drawCalls)
	{
		renderpass_->begin(cb, framebuffer_->frame(currentFrame), scissor, VK_SUBPASS_CONTENTS_INLINE);
		pipeline_->bind(cb);
		vkCmdSetViewport(cb.commandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(cb.commandBuffer(), 0, 1, &scissor);

		drawCalls();

		renderpass_->end(cb);
	}
}