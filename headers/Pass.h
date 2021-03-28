#pragma once
#include "vulkan/vulkan.hpp"
#include "Shader.h"
#include "Pipeline.h"
#include "vulkanContext.h"
#include "RenderTarget.h"
#include <vector>
#include <unordered_map>
#include <string>
namespace vkn
{
	class Renderpass;
	class Pass
	{
	public:
		struct RenderArea
		{
			VkViewport viewport;
			VkRect2D scissor;
		};

		Pass(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		Pass(Pass&&) = default;
		~Pass() = default;
		void attachToRenderpass(Context& constext, const VkRenderPass& renderpass, const uint32_t subpassIndex);
		void bind(CommandBuffer& cb);
		void rebuild(Context& context);
		void usesColorTarget(RenderTarget& target);
		void usesDepthStencilTarget(RenderTarget& target);
		void consumesTarget(RenderTarget& target);
		std::vector<RenderTargetRef>& colorTargets();
		std::vector<RenderTargetRef>& depthStencilTargets();
		std::vector<RenderTargetRef>& inputTargets();
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
		VkFrontFace frontFace;
		VkCullModeFlags cullMode;
		float lineWidth;
	private:
		bool support3D{ false };
		VkGraphicsPipelineCreateInfo pipelineInfo_{};
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

		std::vector<RenderTargetRef> colorTargets_;
		std::vector<RenderTargetRef> depthStencilTargets_;
		std::vector<RenderTargetRef> inputTargets_;

		std::unique_ptr<Pipeline>pipeline_;
		void buildPipeline(Context& context, const VkRenderPass& renderpass, const uint32_t subpassIndex);
	};
}