#include "..\headers\ShaderTechnique.h"

vkn::ShaderTechnique::ShaderTechnique(vkn::RenderpassBuilder& renderpassBuilder, vkn::PipelineBuilder& pipelineBuilder, vkn::Swapchain& swapchain)
{
	renderpass_ = std::make_unique<vkn::Renderpass>(renderpassBuilder.get());

	pipelineBuilder.subpass = 0;
	pipelineBuilder.renderpass = renderpass_->renderpass();
	pipeline_ = std::make_unique<vkn::Pipeline>(pipelineBuilder.get());
	framebuffer_ = std::make_unique<vkn::Framebuffer>(pipelineBuilder.gpu_, pipelineBuilder.device_, *renderpass_, swapchain);
}

vkn::ShaderTechnique::ShaderTechnique(vkn::RenderpassBuilder& renderpassBuilder, vkn::PipelineBuilder& pipelineBuilder, const VkExtent2D& size)
{
	renderpass_ = std::make_unique<vkn::Renderpass>(renderpassBuilder.get());

	pipelineBuilder.subpass = 0;
	pipelineBuilder.renderpass = renderpass_->renderpass();
	pipeline_ = std::make_unique<vkn::Pipeline>(pipelineBuilder.get());
	
	framebuffer_ = std::make_unique<vkn::Framebuffer>(pipelineBuilder.gpu_, pipelineBuilder.device_, *renderpass_, size);
}

void vkn::ShaderTechnique::setClearColor(const glm::vec3& color)
{
	renderpass_->setClearColor(color);
}

void vkn::ShaderTechnique::updatePipelineTexture(const std::string& resourceName, const VkSampler& sampler, const VkImageView& view, const VkShaderStageFlagBits stage)
{
	pipeline_->updateTexture(resourceName, sampler, view, stage);
}

void vkn::ShaderTechnique::updatePipelineTextures(const std::string& resourceName, const VkSampler& sampler, const std::vector<VkImageView>& views, const VkShaderStageFlagBits stage)
{
	pipeline_->updateTextures(resourceName, sampler, views, stage);
}

VkFramebuffer vkn::ShaderTechnique::framebuffer(const uint32_t index)
{
	return framebuffer_->frame(index);
}
