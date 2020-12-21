#pragma once
#include "vulkan/vulkan.hpp"
#include "Gpu.h"
#include "Device.h"
#include "Shader.h"
#include "Pipeline.h"
#include "RenderpassBuilder.h"

#include <vector>
#include <string>
namespace vkn
{
	class PipelineBuilder
	{
	public:
		struct RenderArea
		{
			VkViewport viewport;
			VkRect2D scissor;
		};

		PipelineBuilder();
		PipelineBuilder(PipelineBuilder&&) = default;
		~PipelineBuilder() = default;
		void preBuild(vkn::Device& device);
		vkn::Pipeline get(vkn::Gpu& gpu, vkn::Device& device);
		void addShaderStage(const VkShaderStageFlagBits stage, const std::string& path);
		void setInputBindingRate(const uint32_t binding, const VkVertexInputRate rate);
		void addAssemblyStage(const VkPrimitiveTopology primitive, const VkBool32 restart = VK_FALSE);
		void addTesselationStage(const uint32_t controlPoints);
		void addViewport(const RenderArea& area);
		void addRaterizationStage(const VkPolygonMode mode);
		void addMultisampleStage(const VkSampleCountFlagBits sampleCount);
		void addDepthStage(const VkCompareOp op, const VkBool32 write = VK_TRUE);
		void addStencilStage(const VkStencilOpState front, const VkStencilOpState back);
		void addColorBlendStage(const VkBool32 logicEnabled = VK_FALSE, const VkLogicOp op = VK_LOGIC_OP_OR);
		void addColorBlendAttachment(const VkPipelineColorBlendAttachmentState& attachment);
		void addDynamicState(const VkDynamicState state);
		void setPolygonMode(const VkPolygonMode mode);
		const std::vector<vkn::Shader::Attachment>& subpassInputAttachments() const;
		const std::vector<vkn::Shader::Attachment>& outputAttachments() const;
		const std::vector<vkn::Shader::Attachment> getColorOutputAttachment() const;

		VkFrontFace frontFace;
		VkCullModeFlags cullMode;
		float lineWidth;
		VkRenderPass renderpass;
		uint32_t subpass;
	private:
		bool support3D{ false };
		VkPipelineVertexInputStateCreateInfo vertexInput_{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI_{};
		VkPipelineTessellationStateCreateInfo tesselationCI_{};
		VkPipelineViewportStateCreateInfo viewportCI_{};
		VkPipelineRasterizationStateCreateInfo rasterizationCI_{};
		VkPipelineMultisampleStateCreateInfo multisampleCI_{};
		VkPipelineDepthStencilStateCreateInfo depthStencilCI_{};
		VkPipelineColorBlendStateCreateInfo colorBlendCI_{};
		VkPipelineDynamicStateCreateInfo dynamicStateCI_{};


		std::vector<RenderArea> renderAreas_{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments_{};
		std::vector<VkDynamicState> dynamicStates_{};
		std::vector<std::pair<VkShaderStageFlagBits, std::string>> shaderStages_;
		std::vector<vkn::Shader> shaders_{};

	};
}