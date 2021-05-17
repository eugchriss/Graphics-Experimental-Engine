#include <string>
#include "../../headers/MaterialInstance.h"
#include "../../headers/impl/vulkan/Material.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

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

void vkn::Material::draw(GeometryMemoryHolder& memoryHolder, TextureMemoryHolder& imageHolder, CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<gee::MaterialInstancePtr>& materialInstances)
{
	assert(std::size(materialInstances) <= 15); //maxDescriptorSetDynamicUniformBuffer
	getPackedTextures_and_transforms(imageHolder, const_cast<std::vector<gee::MaterialInstancePtr>&>(materialInstances));

	pipeline_->updateBuffer("transform_matrices", transformMatrices_);
	pipeline_->updateTextures("colors", sampler_, textureSlots_[TEXTURE_SLOT::COLOR]);
	pipeline_->updateTextures("normals", sampler_, textureSlots_[TEXTURE_SLOT::NORMAL]);
	set_pointLights();
	VkDeviceSize offset{ 0 };
	pipeline_->bind(cb);
	pipeline_->pushConstant(cb, "camera", cameraShaderInfo);
	for (auto i = 0u; i < std::size(materialInstances); ++i)
	{
		pipeline_->pushConstant(cb, "material_index", i);
		for (auto& [geometryRef, transforms] : materialInstances[i]->geometries)
		{
			assert(std::size(transforms) == max_object_per_instance()); 
			auto& geometryMemory = memoryHolder.get(geometryRef.get().hash, geometryRef.get());

			vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &geometryMemory.vertexBuffer.buffer, &offset);
			vkCmdBindIndexBuffer(cb.commandBuffer(), geometryMemory.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			pipeline_->bind_set(cb, PER_MATERIAL_SET, 1, { i *	dynamicAlignment_});
			vkCmdDrawIndexed(cb.commandBuffer(), geometryMemory.indicesCount, std::size(transforms), 0, 0, 0);
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

void vkn::Material::getPackedTextures_and_transforms(TextureMemoryHolder& imageHolder, std::vector<gee::MaterialInstancePtr> & materialInstances)
{
	textureSlots_.clear();
	auto offset = 0u;
	for (auto& materialInstanceRef : materialInstances)
	{
		auto& materialInstance = *materialInstanceRef;

		auto& color = materialInstance.textureSlots.find(TEXTURE_SLOT::COLOR);
		if (color != std::end(materialInstance.textureSlots))
		{
			textureSlots_[TEXTURE_SLOT::COLOR].emplace_back(imageHolder.get(color->second.get().name(), color->second.get()).getView(VK_IMAGE_ASPECT_COLOR_BIT));
		}
		auto& normal = materialInstance.textureSlots.find(TEXTURE_SLOT::NORMAL);
		if (normal != std::end(materialInstance.textureSlots))
		{
			textureSlots_[TEXTURE_SLOT::NORMAL].emplace_back(imageHolder.get(normal->second.get().name(), normal->second.get()).getView(VK_IMAGE_ASPECT_COLOR_BIT));
		}

		offset += materialInstance.copy_geometries_to(std::begin(transformMatrices_) + offset);
		if (offset > std::size(transformMatrices_))
		{
			//TODO: handle cases where 1 pipeline dynamic uniform isn t enough.
			throw std::runtime_error{ "A new pipeline is required. Too many transforms" };
		}
	}
}
