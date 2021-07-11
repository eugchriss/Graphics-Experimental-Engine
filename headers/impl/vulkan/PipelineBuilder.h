#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../headers/ShaderTechnique.h"

#include "Pipeline.h"
#include "renderpass.h"
#include "Shader.h"
#include "vulkanContext.h"

namespace gee
{
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

			PipelineBuilder(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
			PipelineBuilder(PipelineBuilder&&) = default;
			~PipelineBuilder() = default;
			vkn::Pipeline get(Context& context);
			void setShaderVertexStage(const std::string& path);
			void setShaderFragmentStage(const std::string& path);
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
			void set_dynamic_alignments(const std::unordered_map<uint32_t, std::vector<size_t>>& dynamicAlignments);

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
			std::unordered_map<VkShaderStageFlagBits, std::string> shaderStages_;
			std::vector<vkn::Shader> shaders_{};
			std::unordered_map<uint32_t, std::vector<size_t>> setDynamicAlignments_;
		};
	}

	template<class T> struct ResourceLoader;
	template<>
	struct ResourceLoader<vkn::Pipeline>
	{
		static vkn::Pipeline load(vkn::Context& context, const vkn::Renderpass& rp, const gee::ShaderTechnique& technique)
		{
			const_cast<bool&>(technique.isGPUInitialized_) = true;
			vkn::PipelineBuilder builder{ technique.vertexShaderPath_, technique.fragmentShaderPath_};
			builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
			builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
			builder.addDepthStage(VK_COMPARE_OP_LESS_OR_EQUAL);
			builder.addColorBlendStage();
			builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
			builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
			builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
			builder.subpass = static_cast<uint32_t>(technique.passIndex_);
			builder.renderpass = rp();
			builder.set_dynamic_alignments(technique.dynamic_alignments());
			return builder.get(context);
		}
	};
}