#include "..\headers\Material.h"
#include "../headers/vulkan_utils.h"

vkn::Material::Material(Context& context, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const RENDERPASS_USAGE& passUsage):
	context_{context},  builder_{ vertexShaderPath, fragmentShaderPath }, passUsage_{passUsage}
{
	prepare_pipeline(context, passUsage);
}

vkn::Material::Material(Material&& other): context_{other.context_}, builder_{std::move(other.builder_)}, passUsage_{other.passUsage_}
{
	pipeline_ = std::move(other.pipeline_);
	sampler_ = other.sampler_;
	other.sampler_ = VK_NULL_HANDLE;
}

void vkn::Material::bind(CommandBuffer& cb, const VkRenderPass& renderpass)
{
	build_pipeline(renderpass);
	pipeline_->bind(cb);
}

void vkn::Material::draw(CommandBuffer& cb)
{
	vkCmdDraw(cb.commandBuffer(), 3, 1, 0, 0);
}

void vkn::Material::set_sampler(const VkSamplerCreateInfo& samplerInfo)
{
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
