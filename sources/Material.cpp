#include "..\headers\Material.h"
#include "../headers/vulkan_utils.h"

vkn::Material::Material(Context& context, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const RENDERPASS_USAGE& passUsage) :
	context_{ context }, builder_{ vertexShaderPath, fragmentShaderPath }, passUsage_{ passUsage }
{
	prepare_pipeline(context, passUsage);
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	set_sampler(samplerInfo);
}

vkn::Material::Material(Material&& other) : context_{ other.context_ }, builder_{ std::move(other.builder_) }, passUsage_{ other.passUsage_ }
{
	pipeline_ = std::move(other.pipeline_);
	sampler_ = other.sampler_;
	other.sampler_ = VK_NULL_HANDLE;
	textureSlots_ = std::move(other.textureSlots_);
}

vkn::Material::~Material()
{
	if (sampler_ != VK_NULL_HANDLE)
	{
		vkDestroySampler(context_.device->device, sampler_, nullptr);
	}
}

void vkn::Material::bind(const VkRenderPass& renderpass)
{
	build_pipeline(renderpass);
}

void vkn::Material::draw(CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<MaterialInstance>& materialInstances)
{
	assert(std::size(materialInstances) <= 15); //maxDescriptorSetDynamicUniformBuffer
	getPackedTextures_and_transforms(const_cast<std::vector<MaterialInstance>&>(materialInstances));

	pipeline_->updateBuffer("transform_matrices", transformMatrices_);
	pipeline_->updateTextures("colors", sampler_, textureSlots_[TEXTURE_SLOT::COLOR]);

	VkDeviceSize offset{ 0 };
	pipeline_->bind(cb);
	pipeline_->pushConstant(cb, "camera", cameraShaderInfo);
	for (auto i = 0u; i < std::size(materialInstances); ++i)
	{
		pipeline_->pushConstant(cb, "material_index", i);
		for (auto& geometry : materialInstances[i].geometries)
		{
			assert(std::size(geometry.transformMatrices) < 1024); //maxUniformBufferRange / sizeof(mat4)
			vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &geometry.geometryMemoryRef.get().vertexBuffer.buffer, &offset);
			vkCmdBindIndexBuffer(cb.commandBuffer(), geometry.geometryMemoryRef.get().indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			pipeline_->bind_set(cb, PER_MATERIAL_SET, 1, {0 * 1024 * sizeof(glm::mat4)});
			vkCmdDrawIndexed(cb.commandBuffer(), geometry.geometryMemoryRef.get().indicesCount, std::size(geometry.transformMatrices), 0, 0, 0);
		}
	}
}

void vkn::Material::set_sampler(const VkSamplerCreateInfo& samplerInfo)
{
	vkn::error_check(vkCreateSampler(context_.device->device, &samplerInfo, nullptr, &sampler_), "Failed to create the sampler");
}

vkn::RENDERPASS_USAGE vkn::Material::pass_usage() const
{
	return passUsage_;
}


void vkn::Material::build_pipeline(const VkRenderPass& renderpass)
{
	if (builder_.renderpass != renderpass || !pipeline_)
	{
		builder_.renderpass = renderpass;
		pipeline_ = std::make_unique<vkn::Pipeline>(builder_.get(context_));
	}
}

void vkn::Material::prepare_pipeline(Context& context, const RENDERPASS_USAGE& passUsage)
{
	builder_.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder_.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder_.addDepthStage(VK_COMPARE_OP_LESS);
	builder_.addColorBlendStage();
	builder_.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder_.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder_.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder_.subpass = passUsage;
}

void vkn::Material::getPackedTextures_and_transforms(std::vector<MaterialInstance>& materialInstances)
{
	transformMatrices_.clear();
	textureSlots_.clear();
	for (auto& materialInstance : materialInstances)
	{
		textureSlots_[TEXTURE_SLOT::COLOR].emplace_back(materialInstance.textureSlots[TEXTURE_SLOT::COLOR]);
		for (const auto& geometry : materialInstance.geometries)
		{
			std::copy(std::begin(geometry.transformMatrices), std::end(geometry.transformMatrices), std::back_inserter(transformMatrices_));
		}
	}
}
