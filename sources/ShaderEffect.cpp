#include "..\headers\ShaderEffect.h"

std::unordered_map<vkn::ShaderEffect::Requirement, std::string> vkn::ShaderEffect::requirement_map{};
vkn::ShaderEffect::ShaderEffect(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath):
	name_{ name },
	vertexShaderPath_ {
	vertexShaderPath
},
	fragmentShaderPath_{fragmentShaderPath}
{
	pipelineBuilder_.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderPath);
	pipelineBuilder_.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderPath);
	pipelineBuilder_.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	pipelineBuilder_.addRaterizationStage(VK_POLYGON_MODE_FILL);
	pipelineBuilder_.addDepthStage(VK_COMPARE_OP_LESS);
	pipelineBuilder_.addColorBlendStage();
	pipelineBuilder_.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	pipelineBuilder_.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineBuilder_.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	
	requirement_map[Requirement::Texture] = "textures";
	requirement_map[Requirement::Camera] = "Camera";
	requirement_map[Requirement::Transform] = "Model_Matrix";
	requirement_map[Requirement::Material] = "Materials";
	requirement_map[Requirement::Skybox] = "skybox";
	requirement_map[Requirement::Light] = "PointLights";
}

const std::string& vkn::ShaderEffect::name() const
{
	return name_;
}

void vkn::ShaderEffect::setViewport(const float x, const float y, const float width, const float height)
{
	viewport_.x = x;
	viewport_.y = y;
	viewport_.width = width;
	viewport_.height = height;
	if (hasDepthBuffer())
	{
		viewport_.minDepth = 0.0f;
		viewport_.maxDepth = 1.0f;
	}
	
	glm::u32vec2 origin{ static_cast<uint32_t>(x), static_cast<uint32_t>(y)};
	glm::u32vec2 extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
	setScissor(origin, extent);
}

void vkn::ShaderEffect::setScissor(const glm::u32vec2& origin, const glm::u32vec2& extent)
{
	scissor_.offset.x = origin.x;
	scissor_.offset.x = origin.y;
	scissor_.extent.width = extent.x;
	scissor_.extent.height = extent.y;
}

void vkn::ShaderEffect::render(vkn::CommandBuffer& cb, MeshHolder_t& geometryHolder, const gee::Occurence<Hash_t>& geometries) const
{
	uint32_t instanceIndex{ 0 };
	for (const auto [meshHashKey, instanceCount] : geometries)
	{
		VkDeviceSize offset{ 0 };
		auto& memoryLocation = geometryHolder.get(meshHashKey);

		vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &memoryLocation.vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(cb.commandBuffer(), memoryLocation.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cb.commandBuffer(), memoryLocation.indicesCount, instanceCount, 0, 0, instanceIndex);
		instanceIndex += instanceCount;
	}
}

void vkn::ShaderEffect::setPolygonMode(const VkPolygonMode mode)
{
	polyMode_ = mode;
	pipelineBuilder_.setPolygonMode(polyMode_);
}

void vkn::ShaderEffect::setLineWidth(const float width)
{
	lineWidth_ = width;
}

void vkn::ShaderEffect::setSampleCount(const VkSampleCountFlagBits count)
{
	sampleCount_ = count;
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

bool vkn::ShaderEffect::hasDepthBuffer() const
{
	return hasDepthBuffer_;
}

bool vkn::ShaderEffect::hasStencilBuffer() const
{
	return hasStencilBuffer_;
}

const std::vector<vkn::Shader::Attachment>& vkn::ShaderEffect::outputAttachments() const
{
	return outputAttachments_;
}

const std::vector<vkn::Shader::Attachment>& vkn::ShaderEffect::subpassInputAttachments() const
{
	return subpassInputAttachments_;
}

const uint32_t vkn::ShaderEffect::getRequirement() const
{
	return requirement_;
}

void vkn::ShaderEffect::preload(vkn::Device& device)
{
	pipelineBuilder_.preBuild(device);
	outputAttachments_ = pipelineBuilder_.outputAttachments();
	subpassInputAttachments_ = pipelineBuilder_.subpassInputAttachments();
}

void vkn::ShaderEffect::active(vkn::Gpu& gpu, vkn::Device& device, const VkRenderPass& renderpass, const uint32_t subpass)
{
	if (renderpass != renderpass_)
	{
		pipelineBuilder_.renderpass = renderpass;
		pipelineBuilder_.subpass = subpass;
		renderingIndex_ = subpass;
		if (pipeline_)
		{
			pipelineBuilder_.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderPath_);
			pipelineBuilder_.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderPath_);
			pipelineBuilder_.preBuild(device);
		}
		pipeline_ = std::make_unique<vkn::Pipeline>(pipelineBuilder_.get(gpu, device));
		renderpass_ = renderpass;

		setRequirements(pipeline_->uniforms());
	}
}

void vkn::ShaderEffect::bind(vkn::CommandBuffer& cb)
{
	pipeline_->bind(cb);
	vkCmdSetViewport(cb.commandBuffer(), 0, 1, &viewport_);
	vkCmdSetScissor(cb.commandBuffer(), 0, 1, &scissor_);
}

const uint32_t vkn::ShaderEffect::index() const
{
	return renderingIndex_;
}

void vkn::ShaderEffect::setRequirements(const std::vector<vkn::Pipeline::Uniform>& uniforms)
{
	for (const auto& [requirement, name] : requirement_map)
	{
		if (std::find_if(std::begin(uniforms), std::end(uniforms), [&](const auto& uniform) { return uniform.name == name; }) != std::end(uniforms))
		{
			requirement_ |= requirement;
		}
	}
}
