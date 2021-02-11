#include "../headers/Pipeline.h"
#include <cassert>


vkn::Pipeline::Pipeline(Context& context, const VkPipeline pipeline, std::vector<vkn::Shader>&& shaders) : context_{ context }, pipeline_{ pipeline }, shaders_{ std::move(shaders) }
{
	if (!context_.pushDescriptorEnabled())
	{
		throw std::runtime_error{"Push descriptor'device extension needs to be enabled" };
	}
	layout_ = std::make_unique<vkn::PipelineLayout>(*context.device, shaders_);

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(context.gpu->device, &props);
	auto alignment = props.limits.minUniformBufferOffsetAlignment;
	auto align = [&alignment](VkDeviceSize offset) {
		if (offset % alignment != 0)
		{
			offset += alignment - (offset % alignment);
		}
		return offset;
	};

	VkDeviceSize size{ 0 };
	const auto& layouts = layout_->layouts();
	for (const auto& shader : shaders_)
	{
		//uniforms
		const auto& shaderBindings = shader.bindings();
		for (const auto& binding : shaderBindings)
		{
			Uniform uniform{};
			uniform.name = binding.name;
			uniform.setLayout = layouts[binding.set];
			uniform.binding = binding.layoutBinding.binding;
			uniform.offset = size;
			uniform.size = binding.size;
			uniform.range = binding.range;
			uniform.type = binding.layoutBinding.descriptorType;

			uniforms_.push_back(uniform);
			size += uniform.range;
			size = align(size);
		}
		const auto& subpassInputBindings = shader.subpassInputBindings();
		for (const auto& binding : subpassInputBindings)
		{
			Uniform uniform{};
			uniform.name = binding.name;
			uniform.setLayout = layouts[binding.set];
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
	//create the buffer and the memory, then bind both
	if (size == 0)
	{
		size = 1;
	}
	buffer_ = std::make_unique<vkn::Buffer>(context_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size);

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(context_.device->device, buffer_->buffer, &requirements);
	memory_ = std::make_unique<vkn::DeviceMemory>(*context_.gpu, *context_.device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits, requirements.size);

	buffer_->bind(*memory_);

	vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetInstanceProcAddr(context_.instance->instance, "vkCmdPushDescriptorSetKHR"));
}

vkn::Pipeline::Pipeline(vkn::Pipeline&& other) : context_{other.context_}
{
	pipeline_ = other.pipeline_;
	layout_ = std::move(other.layout_);
	memory_ = std::move(other.memory_);
	buffer_ = std::move(other.buffer_);

	shaders_ = std::move(other.shaders_);
	uniforms_ = std::move(other.uniforms_);
	pushConstants_ = std::move(other.pushConstants_);

	other.pipeline_ = VK_NULL_HANDLE;
	vkCmdPushDescriptorSetKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetInstanceProcAddr(context_.instance->instance, "vkCmdPushDescriptorSetKHR"));
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
	vkCmdBindPipeline(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
}


void vkn::Pipeline::updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such uniform name within this pipeline" };
	}
	auto imageInfo = std::make_shared<VkDescriptorImageInfo>();
	imageInfo->sampler = sampler;
	imageInfo->imageView = view;
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos_.emplace_back(imageInfo);

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.pImageInfo = imageInfo.get();
	write.descriptorCount = uniform->size;

	uniformsWrites_.emplace_back(write);
}

void vkn::Pipeline::updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such uniform name within this pipeline" };
	}

	auto imagesInfos = std::make_shared<std::vector<VkDescriptorImageInfo>>();
	imagesInfos->reserve(std::size(views));
	for (const auto view : views)
	{
		imagesInfos->emplace_back(VkDescriptorImageInfo{ sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	}
	imagesInfos_.emplace_back(imagesInfos);
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.pImageInfo = std::data(*imagesInfos);
	write.descriptorCount = std::size(views);

	uniformsWrites_.emplace_back(write);
}

const std::vector<vkn::Pipeline::Uniform> vkn::Pipeline::uniforms() const
{
	return uniforms_;
}

void vkn::Pipeline::updateUniforms(CommandBuffer& cb)
{
	assert(cb.isRecording() && "Command buffer needs to be in recording state");
	//need to sort uniform write to make a push per set
	vkCmdPushDescriptorSetKHR(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout_->layout, 0, std::size(uniformsWrites_), std::data(uniformsWrites_));
	uniformsWrites_.clear();
	imageInfos_.clear();
	imagesInfos_.clear();
	bufferInfos_.clear();
}