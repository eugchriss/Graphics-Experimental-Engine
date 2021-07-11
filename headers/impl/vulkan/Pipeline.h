#pragma once
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Buffer.h"
#include "CommandBuffer.h"
#include "DeviceMemory.h"
#include "PipelineLayout.h"
#include "Shader.h"
#include "vulkanContext.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		class Image;
		class Pipeline
		{
			using Set = uint32_t;
			using Alignments = std::vector<size_t>;
		public:

			enum UNIFORM_TYPE
			{
				NON_DYNAMIC,
				DYNAMIC
			};

			struct Uniform
			{
				std::string name;
				VkDescriptorType type;
				VkDescriptorSet set;
				uint32_t binding;
				VkDeviceSize offset;
				VkDeviceSize size;
				VkDeviceSize range;
				UNIFORM_TYPE dynamicType;
			};

			struct DescriptorSetOffsets
			{
				uint32_t set;
				std::vector<size_t> offsets;
			};

			Pipeline(Context& context, const VkPipeline pipeline, vkn::PipelineLayout&& pipelineLayout, std::vector<vkn::Shader>&& shaders, std::unordered_map<Set, Alignments>&& dynamicAlignments);
			Pipeline(Pipeline&& other);
			~Pipeline();
			void bind(vkn::CommandBuffer& cb);
			void bind_set(CommandBuffer& cb, std::vector<DescriptorSetOffsets>& descriptorSetOffsets);
			const std::vector<Uniform> uniforms() const;
			void push_constant(vkn::CommandBuffer& cb, const gee::ShaderValue& val);

			template<class T>
			void updateBuffer(const std::string& resourceName, const T& datas);
			void update_shader_value(const gee::ShaderValue& val);
			void update_shader_texture(const vkn::ShaderTexture& tex);
			void update_shader_array_texture(const vkn::ShaderArrayTexture& tex);
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
			std::unordered_map<UNIFORM_TYPE, VkDeviceSize> uniformTypeBufferOffsets_;
			std::vector<VkWriteDescriptorSet> uniformsWrites_;
			std::vector<std::shared_ptr<VkDescriptorImageInfo>> imageInfos_;
			std::vector<std::shared_ptr<std::vector<VkDescriptorImageInfo>>> imagesInfos_;
			std::vector<std::shared_ptr<VkDescriptorBufferInfo>> bufferInfos_;
			std::unique_ptr<Image> dummyImage_;
			std::unordered_map<Set, Alignments> setDynamicAlignments_{};
			void createBuffers(const VkDeviceSize nonDynamicSize, const VkDeviceSize dynamicSize);
		};

		template<class T>
		inline void Pipeline::updateBuffer(const std::string& resourceName, const T& datas)
		{
			auto uniform = std::find_if(std::begin(uniforms_), std::end(uniforms_), [&](auto& uniform) { return uniform.name == resourceName; });
			if (uniform == std::end(uniforms_))
			{
				throw std::runtime_error{ "There is no such uniform name within this pipeline" };
			}

			auto bufferInfo = std::make_shared<VkDescriptorBufferInfo>();
			bufferInfo->buffer = buffer_->buffer;
			//update the memory
			if (uniform->dynamicType == UNIFORM_TYPE::DYNAMIC)
			{
				buffer_->update(uniformTypeBufferOffsets_[DYNAMIC] + uniform->offset, datas);

				//update the descriptor
				bufferInfo->offset = uniform->offset;
				bufferInfo->range = uniform->range;
			}
			else
			{
				buffer_->update(uniformTypeBufferOffsets_[uniform->dynamicType] + uniform->offset, datas);
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
	}
}
