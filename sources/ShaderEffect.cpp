#include "..\headers\ShaderEffect.h"
#include "../headers/RenderpassBuilder.h"
#include "../headers/PipelineBuilder.h"

vkn::ShaderEffect::ShaderEffect(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, Function_t& function, const VkImageLayout finalLayout):
	vertexShaderPath_{vertexShaderPath},
	fragmentShaderPath_{fragmentShaderPath},
	drawCall_{function},
	finalLayout_{finalLayout}
{
}

void vkn::ShaderEffect::execute(vkn::CommandBuffer& cb, const VkViewport& viewport, const VkRect2D& renderArea, const std::unordered_map<Hash_t, vkn::MemoryLocation>& memoryLocations, const std::unordered_map<Hash_t, uint32_t>& drawables)
{
	drawCall_(cb, viewport, renderArea, memoryLocations, drawables);
}

void vkn::ShaderEffect::setPolygonMode(const VkPolygonMode mode)
{
	polyMode_ = mode;
}

void vkn::ShaderEffect::setLineWidth(const float width)
{
	lineWidth_ = width;
}

void vkn::ShaderEffect::setSampleCount(const VkSampleCountFlagBits count)
{
	sampleCount_ = count;
}

void vkn::ShaderEffect::setLoadOp(const VkAttachmentLoadOp op)
{
	loadOp_ = op;
}

void vkn::ShaderEffect::setStoreOp(const VkAttachmentStoreOp op)
{
	storeOp_ = op;
}

void vkn::ShaderEffect::updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage)
{
	assert(pipeline_ && "The pipeline should be created first");
	pipeline_->updateTexture(resourceName, sampler, view, stage);
}

void vkn::ShaderEffect::updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage)
{
	assert(pipeline_ && "The pipeline should be created first");
	pipeline_->updateTextures(resourceName, sampler, views, stage);
}

const vkn::ShaderEffect::Requirement vkn::ShaderEffect::getRequirement() const
{
	return requirement;
}

void vkn::ShaderEffect::begin(vkn::CommandBuffer& cb, const VkFramebuffer& fb, const VkRect2D& renderArea, const VkSubpassContents subpassContent)
{
	renderpass_->begin(cb, fb, renderArea, subpassContent);
}

void vkn::ShaderEffect::end(vkn::CommandBuffer& cb)
{
	renderpass_->end(cb);
}

void vkn::ShaderEffect::create(vkn::Gpu& gpu, vkn::Device& device, const VkImageLayout initialLayout)
{
	vkn::RenderpassBuilder renderpassBuilder{ device };

	auto pipelineBuilder = vkn::PipelineBuilder::getDefault3DPipeline(gpu, device, vertexShaderPath_, fragmentShaderPath_);
	pipelineBuilder.lineWidth = lineWidth_;
	pipelineBuilder.addRaterizationStage(polyMode_);
	pipelineBuilder.addMultisampleStage(sampleCount_);
	pipelineBuilder.buildSubpass(renderpassBuilder, initialLayout, finalLayout_, loadOp_, storeOp_);
	pipeline_ = std::make_unique<vkn::Pipeline>(pipelineBuilder.get());
	renderpass_ = std::make_unique<vkn::Renderpass>(renderpassBuilder.get());

	attachmentFormats_ = std::move(renderpassBuilder.attachmentFormats());
}
