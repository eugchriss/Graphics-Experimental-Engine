#include <string>
#include "../../headers/impl/vulkan/Material.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;
uint32_t vkn::Material::dynamicAlignment_{};

vkn::Material::Material(Context& context, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const RENDERPASS_USAGE& passUsage) :
	context_{ context }, builder_{ vertexShaderPath, fragmentShaderPath }, passUsage_{ passUsage }
{
	gee::hash_combine(hash_, vertexShaderPath, fragmentShaderPath);
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

	const auto limits = context_.gpu->properties().limits;
    dynamicAlignment_ = limits.maxUniformBufferRange;
	transformMatrices_.resize(max_object_per_instance()* limits.maxDescriptorSetUniformBuffersDynamic);
}

vkn::Material::Material(Material&& other) : context_{ other.context_ }, builder_{ std::move(other.builder_) }, passUsage_{ other.passUsage_ }
{
	pipeline_ = std::move(other.pipeline_);
	sampler_ = other.sampler_;
	other.sampler_ = VK_NULL_HANDLE;
	textureSlots_ = std::move(other.textureSlots_);
	hash_ = other.hash_;
	dynamicAlignment_ = other.dynamicAlignment_;
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

void vkn::Material::set_sampler(const VkSamplerCreateInfo& samplerInfo)
{
	vkn::error_check(vkCreateSampler(context_.device->device, &samplerInfo, nullptr, &sampler_), "Failed to create the sampler");
}

vkn::RENDERPASS_USAGE vkn::Material::pass_usage() const
{
	return passUsage_;
}

const size_t vkn::Material::hash() const
{
	return hash_;
}

void vkn::Material::build_pipeline(const VkRenderPass& renderpass)
{
	if (builder_.renderpass != renderpass || !pipeline_)
	{
		builder_.renderpass = renderpass;
		pipeline_ = std::make_unique<vkn::Pipeline>(builder_.get(context_));
	}
}

void vkn::Material::set_pointLights()
{
	if (!std::empty(pointLights_))
	{
		pipeline_->updateBuffer("PointLights", pointLights_);
	}
}

const uint32_t vkn::Material::max_object_per_instance()
{
	return dynamicAlignment_ / sizeof(glm::mat4);
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