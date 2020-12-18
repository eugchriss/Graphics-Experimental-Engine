#include "../headers/Pipeline.h"
#include <cassert>

vkn::Pipeline::Pipeline(vkn::Gpu& gpu, vkn::Device& device, const VkPipeline pipeline, std::vector<vkn::Shader>&& shaders) : gpu_{ gpu }, device_{ device }, pipeline_{ pipeline }, shaders_{ std::move(shaders) }
{
	layout_ = std::make_unique<vkn::PipelineLayout>(device_, shaders_);
	createPool();
	createSets();

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(gpu_.device, &props);
	auto alignment = props.limits.minUniformBufferOffsetAlignment;
	auto align = [&alignment](VkDeviceSize offset) {
		if (offset % alignment != 0)
		{
			offset += alignment - (offset % alignment);
		}
		return offset;
	};

	VkDeviceSize size{ 0 };
	for (const auto& shader : shaders_)
	{
		//uniforms
		const auto& shaderBindings = shader.bindings();
		for (const auto& binding : shaderBindings)
		{
			Uniform uniform{};
			uniform.name = binding.name;
			uniform.set = sets_[binding.set];
			uniform.binding = binding.layoutBinding.binding;
			uniform.offset = size;
			uniform.size = binding.size;
			uniform.range = binding.range;
			uniform.type = binding.layoutBinding.descriptorType;

			uniforms_.push_back(uniform);
			size += uniform.range;
			size = align(size);
		}

		//push constants
		const auto& pushConstants = shader.pushConstants();
		std::copy(std::begin(pushConstants), std::end(pushConstants), std::back_inserter(pushConstants_));
	}
	//create the buffer and the memory, then bind both
	{
		buffer_ = std::make_unique<vkn::Buffer>(device_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size);

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(device_.device, buffer_->buffer, &requirements);
		memory_ = std::make_unique<vkn::DeviceMemory>(gpu_, device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits, requirements.size);

		buffer_->bind(*memory_);
	}

}

vkn::Pipeline::Pipeline(vkn::Pipeline&& other): gpu_{other.gpu_}, device_{other.device_}
{
	pipeline_ = other.pipeline_;
	layout_ = std::move(other.layout_);
	descriptorPool_ = other.descriptorPool_;
	sets_ = std::move(other.sets_);
	memory_ = std::move(other.memory_);
	buffer_ = std::move(other.buffer_);

	shaders_ = std::move(other.shaders_);
	uniforms_ = std::move(other.uniforms_);
	pushConstants_ = std::move(other.pushConstants_);

	other.pipeline_ = VK_NULL_HANDLE;
	other.descriptorPool_ = VK_NULL_HANDLE;
}

vkn::Pipeline::~Pipeline()
{
	if (pipeline_ != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device_.device, pipeline_, nullptr);
	}
	if (!std::empty(sets_))
	{
		vkFreeDescriptorSets(device_.device, descriptorPool_, std::size(sets_), std::data(sets_));
	}
	if (descriptorPool_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device_.device, descriptorPool_, nullptr);
	}
}

void vkn::Pipeline::bind(vkn::CommandBuffer& cb)
{
	vkCmdBindPipeline(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
	vkCmdBindDescriptorSets(cb.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout_->layout, 0, std::size(sets_), std::data(sets_), 0, nullptr);
}


void vkn::Pipeline::updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such uniform name within this pipeline" };
	}
	VkDescriptorImageInfo imageInfo{ sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = uniform->set;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.pImageInfo = &imageInfo;
	write.descriptorCount = uniform->size;
	vkUpdateDescriptorSets(device_.device, 1, &write, 0, nullptr);
}

void vkn::Pipeline::updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage)
{
	auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
	if (uniform == std::end(uniforms_))
	{
		throw std::runtime_error{ "There is no such uniform name within this pipeline" };
	}
	std::vector<VkDescriptorImageInfo> imagesInfos;
	imagesInfos.reserve(std::size(views));

	for (const auto view : views)
	{
		imagesInfos.emplace_back(VkDescriptorImageInfo{ sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
	}
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = uniform->set;
	write.dstBinding = uniform->binding;
	write.descriptorType = uniform->type;
	write.pImageInfo = std::data(imagesInfos);
	write.descriptorCount = std::size(views);
	vkUpdateDescriptorSets(device_.device, 1, &write, 0, nullptr);
}

const std::vector<vkn::Pipeline::Uniform> vkn::Pipeline::uniforms() const
{
	return uniforms_;
}

const std::vector<vkn::Shader::Attachment>& vkn::Pipeline::outputAttachments() const
{
	auto fragmentShader = std::find_if(std::begin(shaders_), std::end(shaders_), [](const auto& shader) { return shader.stage() == VK_SHADER_STAGE_FRAGMENT_BIT; });
	if (fragmentShader == std::end(shaders_))
	{
		throw std::runtime_error{ "The pipeline requires a fragment shader" };
	}

	return fragmentShader->outputAttachments();
}

void vkn::Pipeline::createSets()
{
	auto layouts = layout_->layouts();
	VkDescriptorSetAllocateInfo setInfo{};
	setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setInfo.pNext = nullptr;
	setInfo.descriptorPool = descriptorPool_;
	setInfo.pSetLayouts = std::data(layouts);
	setInfo.descriptorSetCount = std::size(layouts);

	sets_.resize(std::size(layouts));
	vkn::error_check(vkAllocateDescriptorSets(device_.device, &setInfo, std::data(sets_)), "Failed to allocate the pipeline sets");
}

const std::vector<VkDescriptorPoolSize> vkn::Pipeline::getPoolSizes() const
{
	std::vector<VkDescriptorPoolSize> poolSize;
	for (const auto& shader : shaders_)
	{
		auto sizes = shader.poolSize();
		for (const auto& size : sizes)
		{
			auto pool = std::find_if(std::begin(poolSize), std::end(poolSize), [&](auto& pool) { return pool.type == size.type; });
			if (pool == std::end(poolSize))
			{
				poolSize.push_back(size);
			}
			else
			{
				pool->descriptorCount += size.descriptorCount;
			}
		}
	}
	return poolSize;
}

void vkn::Pipeline::createPool()
{
	auto poolSizes = getPoolSizes();
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = std::data(poolSizes);

	vkn::error_check(vkCreateDescriptorPool(device_.device, &poolInfo, nullptr, &descriptorPool_), "Failed to create the descriptor pool");
}