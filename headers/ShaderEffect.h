#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "pipeline.h"
#include "PipelineBuilder.h"
#include "Renderpass.h"
#include "CommandBuffer.h"
#include "MeshMemoryLocation.h"
#include "Drawable.h"
#include "ResourceHolder.h"
#include "TextureImageFactory.h"

using Hash_t = size_t;
using MeshHolder_t = gee::ResourceHolder<vkn::MeshMemoryLocationFactory, vkn::MeshMemoryLocation, Hash_t>;
namespace vkn
{
	template <typename T> using Ptr = std::unique_ptr<T>;
	class Framebuffer;
	class ShaderEffect
	{
	public:
		enum Requirement
		{
			Texture = 1,
			Camera = 2,
			Transform = 4,
			Material = 8,
			Light = 16,
			Skybox = 32
		};
		ShaderEffect(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		ShaderEffect(ShaderEffect&&) = default;
		const std::string& name() const;
		void setViewport(const float x, const float y, const float width, const float height);
		void setScissor(const glm::u32vec2& origin, const glm::u32vec2& extent);
		void render(vkn::CommandBuffer& cb, MeshHolder_t& geometryHolder, const gee::Occurence<Hash_t>& geometries) const;
		void setPolygonMode(const VkPolygonMode mode);
		void setLineWidth(const float width);
		void setSampleCount(const VkSampleCountFlagBits count);
		void updateTexture(const std::string& resourceName, const VkSampler sampler, const VkImageView view, const VkShaderStageFlagBits stage);
		void updateTextures(const std::string& resourceName, const VkSampler sampler, const std::vector<VkImageView> views, const VkShaderStageFlagBits stage);
		bool hasDepthBuffer() const;
		bool hasStencilBuffer() const;
		const std::vector<Shader::Attachment>& outputAttachments() const;
		const std::vector<Shader::Attachment>& subpassInputAttachments() const;
		const uint32_t getRequirement() const;
		void preload(vkn::Device& device);
		void active(vkn::Gpu& gpu, vkn::Device& device, const VkRenderPass& renderpass, const uint32_t subpass);
		void bind(vkn::CommandBuffer& cb);
		const uint32_t index() const;
		template<class T>
		void pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas);
		template<class T>
		void updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage);

		static std::unordered_map<Requirement, std::string> requirement_map;
		friend class vkn::Framebuffer;
	private:
		std::string name_;
		Ptr<vkn::Pipeline> pipeline_;
		vkn::PipelineBuilder pipelineBuilder_;
		VkViewport viewport_{};
		VkRect2D scissor_{};
		VkPolygonMode polyMode_{ VK_POLYGON_MODE_FILL };
		VkSampleCountFlagBits sampleCount_{ VK_SAMPLE_COUNT_1_BIT };
		float lineWidth_{ 1.0f };
		bool hasDepthBuffer_{ true };
		bool hasStencilBuffer_{ false };
		std::vector<VkFormat> attachmentFormats_;
		uint32_t renderingIndex_{};
		VkRenderPass renderpass_;
		uint32_t requirement_{};
		const std::string vertexShaderPath_;
		const std::string fragmentShaderPath_;
		std::vector<vkn::Shader::Attachment> outputAttachments_;
		std::vector<vkn::Shader::Attachment> subpassInputAttachments_;

		void setRequirements(const std::vector<vkn::Pipeline::Uniform>& uniforms);
	};
	template<class T>
	inline void ShaderEffect::pushConstant(vkn::CommandBuffer& cb, const std::string& name, const T& datas)
	{
		assert(pipeline_ && "The pipeline should be created first");
		pipeline_->pushConstant(cb, name, datas);
	}
	template<class T>
	inline void ShaderEffect::updateBuffer(const std::string& resourceName, const T& datas, const VkShaderStageFlagBits stage)
	{
		assert(pipeline_ && "The pipeline should be created first");
		pipeline_->updateBuffer(resourceName, datas, stage);
	}
}