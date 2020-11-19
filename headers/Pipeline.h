#pragma once
#include "vulkan/vulkan.hpp"
#include "Gpu.h"
#include "Device.h"
#include "Shader.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "PipelineLayout.h"
#include "CommandBuffer.h"

#include <vector>
#include <memory>
#include <algorithm>
namespace vkn
{
	class Pipeline
	{
	public:
		Pipeline(vkn::Gpu& gpu, vkn::Device& device, const VkPipeline pipeline, std::vector<vkn::Shader>&& shaders);
		Pipeline(Pipeline&& other);
		~Pipeline();
		void bind(vkn::CommandBuffer& cb);
		void updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage);
		void updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage);

		template<class T>
		void pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas);
		template<class T>
		void updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage);

	private:
		struct Uniform
		{
			std::string name;
			VkDescriptorType type;
			VkDescriptorSet set;
			uint32_t binding;
			VkDeviceSize offset;
			VkDeviceSize size;
			VkDeviceSize range;
		};

		vkn::Gpu& gpu_;
		vkn::Device& device_;
		VkPipeline pipeline_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::PipelineLayout> layout_;
		std::vector<vkn::Shader> shaders_;
		std::vector<VkDescriptorSet> sets_;
		VkDescriptorPool descriptorPool_{ VK_NULL_HANDLE };
		std::vector<Uniform> uniforms_;
		std::vector<vkn::Shader::PushConstant> pushConstants_;
		std::unique_ptr<vkn::DeviceMemory> memory_;
		std::unique_ptr<vkn::Buffer> buffer_;

		void createPool();
		void createSets();
		const std::vector<VkDescriptorPoolSize> getPoolSizes() const;
	};

	template<class T>
	inline void Pipeline::updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage)
	{
		auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
		if (uniform == std::end(uniforms_))
		{
			throw std::runtime_error{ "There is no such uniform name within this pipeline" };
		}
		//update the memory
		buffer_->update(uniform->offset, datas);

		//update the descriptor
		VkDescriptorBufferInfo bufferInfo{ buffer_->buffer, uniform->offset, uniform->range };
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = uniform->set;
		write.dstBinding = uniform->binding;
		write.descriptorType = uniform->type;
		write.descriptorCount = uniform->size;
		write.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(device_.device, 1, &write, 0, nullptr);
	}

	template<class T>
	void vkn::Pipeline::pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas)
	{
		auto result = std::find_if(std::begin(pushConstants_), std::end(pushConstants_), [&](const auto& pc) { return pc.name == name; });
		if (result == std::end(pushConstants_))
		{
			throw std::runtime_error{ "There is no push constant with that name" };
		}
		else
		{
			vkCmdPushConstants(cb.commandBuffer(), layout_->layout, result->stageFlag, result->offsets[0], result->size, &datas);
		}
	}
}
