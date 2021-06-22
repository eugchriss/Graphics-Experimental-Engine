#include <cassert>

#include "../../headers/impl/vulkan/CommandPool.h"
#include "../../headers/impl/vulkan/Image.h"
#include "../../headers/impl/vulkan/Pipeline.h"

using namespace gee;
vkn::Pipeline::Pipeline(Context& context, const VkPipeline pipeline, vkn::PipelineLayout&& pipelineLayout, std::vector<vkn::Shader>&& shaders) : context_{ context }, pipeline_{ pipeline }, layout_{ std::move(pipelineLayout) }, shaders_{ std::move(shaders) }
{
	dummyImage_ = std::make_unique<vkn::Image>(context_, VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R16_SFLOAT, VkExtent3D{ 1,1,1 });
	{
		vkn::CommandPool pool{ context, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
		auto& cb = pool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		cb.begin();
		dummyImage_->transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		cb.end();
		auto transitionFinished = context_.transferQueue->submit(cb);
		transitionFinished->wait();
	}

	VkDeviceSize size{ 0 };
	VkDeviceSize dynamicSize{ 0 };
	const auto& descriptorSets = layout_.sets();
	for (const auto& shader : shaders_)
	{
		//uniforms
		const auto& shaderBindings = shader.bindings();
		for (const auto& binding : shaderBindings)
		{
			Uniform uniform{};
			uniform.name = binding.name;
			uniform.set = descriptorSets[binding.set];
			uniform.binding = binding.layoutBinding.binding;
			uniform.offset = size;
			uniform.size = binding.size;
			uniform.range = binding.range;
			uniform.type = binding.layoutBinding.descriptorType;
			uniform.dynamicType = ((uniform.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (uniform.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) ? UNIFORM_TYPE::DYNAMIC : UNIFORM_TYPE::NON_DYNAMIC;

			if (uniform.dynamicType == UNIFORM_TYPE::DYNAMIC)
			{
				dynamicSize += uniform.range;
			}
			else
			{
				size += uniform.range;
			}
			uniforms_.push_back(uniform);
		}
		const auto& subpassInputBindings = shader.subpassInputBindings();
		for (const auto& binding : subpassInputBindings)
		{
			Uniform uniform{};
			uniform.name = binding.name;
			uniform.set = descriptorSets[binding.set];
			uniform.binding = binding.layoutBinding.binding;
			uniform.offset = size;
			uniform.size = binding.size;
			uniform.range = binding.range;
			uniform.type = binding.layoutBinding.descriptorType;

			uniforms_.push_back(uniform);
		}
		//push constants
		const auto& pushConstants = shader.pushConstants();
		std::copy(std::begin(pushConstants), std::end(pushConstants), std::back_inserter(pushConstants_));
	}

	createBuffers(size, dynamicSize);
}

vkn::Pipeline::Pipeline(vkn::Pipeline&& other) : context_{ other.context_ }, layout_{ std::move(other.layout_) }
{
	pipeline_ = other.pipeline_;
	memory_ = std::move(other.memory_);
	buffer_ = std::move(other.buffer_);
	uniformTypeBufferOffsets_ = std::move(other.uniformTypeBufferOffsets_);

	shaders_ = std::move(other.shaders_);
	uniforms_ = std::move(other.uniforms_);
	pushConstants_ = std::move(other.pushConstants_);
	dummyImage_ = std::move(other.dummyImage_);

	other.pipeline_ = VK_NULL_HANDLE;
}

vkn::Pipeline::~Pipeline()
{
	if (pipeline_ != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(context_.device->device, pipeline_, nullptr);
	}
}

void vkn::Pipeline::bind(vkn::CommandBuffer& cb)
{
	auto& sets = layout_.sets();
	if (!std::empty(sets))
	{
		updateUniforms();
	}
	vkCmdBindPipeline(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}

void vkn::Pipeline::bind_set(CommandBuffer& cb, const uint32_t firstSet, const size_t setCount, const std::vector<uint32_t>& dynamicOffset)
{
	auto& sets = layout_.sets();
	assert(std::size(sets) >= (firstSet + setCount));
	boundSets_.clear();
	for (auto i = firstSet; i < setCount; ++i)
	{
		boundSets_.emplace_back(sets[i]);
	}
	vkCmdBindDescriptorSets(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout_.layout, firstSet, setCount, std::data(boundSets_), std::size(dynamicOffset), std::data(dynamicOffset));
}

const std::vector<vkn::Pipeline::Uniform> vkn::Pipeline::uniforms() const
{
	return uniforms_;
}

void gee::vkn::Pipeline::update_shader_value(const gee::ShaderValue& val)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == val.name; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such uniform name within this pipeline" };
	}

	auto bufferInfo = std::make_shared<VkDescriptorBufferInfo>();
	bufferInfo->buffer = buffer_->buffer;
	//update the memory
	if (uniform->dynamicType == UNIFORM_TYPE::DYNAMIC)
	{
		buffer_->update(uniformTypeBufferOffsets_[DYNAMIC] + uniform->offset, val.address, val.size);

		//update the descriptor
		bufferInfo->offset = uniform->offset;
		bufferInfo->range = uniform->range;
	}
	else
	{
		buffer_->update(uniformTypeBufferOffsets_[uniform->dynamicType] + uniform->offset, val.address, val.size);
	}


	bufferInfos_.emplace_back(bufferInfo);
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = uniform->set;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.descriptorCount = uniform->size;
	write.pBufferInfo = bufferInfo.get();

	uniformsWrites_.emplace_back(write);
}

void gee::vkn::Pipeline::update_shader_texture(const vkn::ShaderTexture& tex)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == tex.name; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such texture name within this pipeline" };
	}
	auto imageInfo = std::make_shared<VkDescriptorImageInfo>();
	imageInfo->sampler = tex.sampler;
	imageInfo->imageView = tex.view;
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos_.emplace_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = uniform->set;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.pImageInfo = imageInfo.get();
	write.descriptorCount = uniform->size;

	uniformsWrites_.emplace_back(write);
}

void gee::vkn::Pipeline::update_shader_array_texture(const vkn::ShaderArrayTexture& tex)
{
	auto arrayTexture = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == tex.name; });
	if (arrayTexture == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such texture name within this pipeline" };
	}

	auto imagesInfos = std::make_shared<std::vector<VkDescriptorImageInfo>>();
	imagesInfos->reserve(arrayTexture->size);

	/* nullDescriptor feature isn t available on macOs so we fill in views with non VK_NULL_HANDLE (aka dummyImage_)*/
	std::vector<VkImageView> views(tex.views);
	auto dummyView = dummyImage_->getView(VK_IMAGE_ASPECT_COLOR_BIT);
	for (auto i = std::size(tex.views); i < arrayTexture->size; ++i)
	{
		views.emplace_back(dummyView);
	}
	for (auto& view : views)
	{
		if (view == VK_NULL_HANDLE)
		{
			view = dummyView;
		}
	}

	for (const auto view : views)
	{
		imagesInfos->emplace_back(VkDescriptorImageInfo{ tex.sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	}
	auto infoSize = std::size(*imagesInfos);

	imagesInfos_.emplace_back(imagesInfos);
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = arrayTexture->set;
	write.dstBinding = arrayTexture->binding;
	write.descriptorType = arrayTexture->type;
	write.pImageInfo = std::data(*imagesInfos);
	write.descriptorCount = std::size(*imagesInfos);

	uniformsWrites_.emplace_back(write);
}

void vkn::Pipeline::updateUniforms()
{
	vkUpdateDescriptorSets(context_.device->device, std::size(uniformsWrites_), std::data(uniformsWrites_), 0, nullptr);
	uniformsWrites_.clear();
	imageInfos_.clear();
	imagesInfos_.clear();
	bufferInfos_.clear();
}

void vkn::Pipeline::createBuffers(const VkDeviceSize nonDynamicSize, const VkDeviceSize dynamicSize)
{
	auto align = [](VkDeviceSize alignment, VkDeviceSize offset) {
		if (offset % alignment != 0)
		{
			offset += alignment - (offset % alignment);
		}
		return offset;
	};

	const auto props = context_.gpu->properties();
	const auto minUniformOffsetAlignment = props.limits.minUniformBufferOffsetAlignment;
	const auto maxDynamicUniformBuffer = props.limits.maxDescriptorSetUniformBuffersDynamic;

	auto dynamic = maxDynamicUniformBuffer * align(minUniformOffsetAlignment, dynamicSize);
	auto nonDynamic = align(minUniformOffsetAlignment, nonDynamicSize);
	uniformTypeBufferOffsets_[UNIFORM_TYPE::NON_DYNAMIC] = 0;
	uniformTypeBufferOffsets_[UNIFORM_TYPE::DYNAMIC] = nonDynamic;

	auto totalSize = dynamic + nonDynamic;
	if (totalSize != 0)
	{
		buffer_ = std::make_unique<vkn::Buffer>(context_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, totalSize);

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(context_.device->device, buffer_->buffer, &requirements);
		memory_ = std::make_unique<vkn::DeviceMemory>(*context_.gpu, *context_.device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits, requirements.size);

		buffer_->bind(*memory_);
	}
}
