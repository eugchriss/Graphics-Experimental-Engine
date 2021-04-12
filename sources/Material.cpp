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
	colorView_ = other.colorView_;
	other.colorView_ = VK_NULL_HANDLE;
}

vkn::Material::~Material()
{
	if (sampler_ != VK_NULL_HANDLE)
	{
		vkDestroySampler(context_.device->device, sampler_, nullptr);
	}
}
void vkn::Material::set_base_color(vkn::Image& image)
{
	colorView_ = image.getView(VK_IMAGE_ASPECT_COLOR_BIT);
}

void vkn::Material::bind(const VkRenderPass& renderpass)
{
	build_pipeline(renderpass);
}

void vkn::Material::draw(CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<GeometyInstances>& geometryInstances)
{
	transformMatrices_.clear();
	for (const auto& geometryInstance : geometryInstances)
	{
		std::copy(std::begin(geometryInstance.transformMatrices), std::end(geometryInstance.transformMatrices), std::back_inserter(transformMatrices_));
	}

	pipeline_->updateBuffer("transform_matrices", transformMatrices_);
	pipeline_->updateTexture("color", sampler_, colorView_);
	pipeline_->bind(cb);
	VkDeviceSize offset{ 0 };
	size_t firstInstance{ 0 };
	pipeline_->pushConstant(cb, "camera", cameraShaderInfo);
	for (auto& geometryInstance : geometryInstances)
	{
		vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &geometryInstance.geometryMemory.vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(cb.commandBuffer(), geometryInstance.geometryMemory.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cb.commandBuffer(), geometryInstance.geometryMemory.indicesCount, std::size(geometryInstance.transformMatrices), 0, 0, firstInstance);		
		firstInstance += std::size(geometryInstance.transformMatrices);
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
