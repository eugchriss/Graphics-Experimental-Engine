#pragma once
#include "vulkan/vulkan.hpp"
#include "vulkanContext.h"
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
	const uint32_t PER_MATERIAL_SET = 0;
	class Image;
	class Pipeline
	{
	public:
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
		Pipeline(Context& context, const VkPipeline pipeline, vkn::PipelineLayout&& pipelineLayout, std::vector<vkn::Shader>&& shaders);
		Pipeline(Pipeline&& other);
		~Pipeline();
		void bind(vkn::CommandBuffer& cb);
		void bind_set(CommandBuffer& cb, const uint32_t firstSet, const size_t setCount, const std::vector<uint32_t>& dynamicOffset = {});
		void updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view);
		void updateTextures(const std::string& resourceName, const VkSampler sampler, std::vector<VkImageView>& views);
		const std::vector<Uniform> uniforms() const;
		template<class T>
		void pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas);
		template<class T>
		void updateBuffer(const std::string& resourceName, const T& datas);
		void updateUniforms();
	private:

		Context& context_;
		VkPipeline pipeline_{ VK_NULL_HANDLE };
		vkn::PipelineLayout layout_;
		std::vector<VkDescriptorSet> boundSets_;
		std::vector<vkn::Shader> shaders_;
		std::vector<Uniform> uniforms_;
		std::vector<vkn::Shader::PushConstant> pushConstants_;
		std::unique_ptr<vkn::DeviceMemory> memory_;
		std::unique_ptr<vkn::Buffer> buffer_;
		std::vector<VkWriteDescriptorSet> uniformsWrites_;
		std::vector<std::shared_ptr<VkDescriptorImageInfo>> imageInfos_;
		std::vector<std::shared_ptr<std::vector<VkDescriptorImageInfo>>> imagesInfos_;
		std::vector<std::shared_ptr<VkDescriptorBufferInfo>> bufferInfos_;
		std::unique_ptr<Image> dummyImage_;
	};

	template<class T>
	inline void Pipeline::updateBuffer(const std::string& resourceName, const T& datas)
	{
		auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
		if (uniform == std::end(uniforms_))
		{
			throw std::runtime_error{ "There is no such uniform name within this pipeline" };
		}
		//update the memory
		buffer_->update(uniform->offset, datas);

		//update the descriptor
		auto bufferInfo = std::make_shared<VkDescriptorBufferInfo>();
		bufferInfo->buffer = buffer_->buffer;
		bufferInfo->offset = uniform->offset;
		bufferInfo->range = uniform->range;
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
			vkCmdPushConstants(cb.commandBuffer(), layout_.layout, result->stageFlag, result->offsets[0], result->size, &datas);
		}
	}
}
