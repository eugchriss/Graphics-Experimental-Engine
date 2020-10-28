#include "../headers/PipelineBuilder.h"
#include "../headers/vulkan_utils.h"

#include <iostream>
#include <stdexcept>
#include <fstream>

vkn::PipelineBuilder::PipelineBuilder(vkn::Gpu& gpu, vkn::Device& device) : gpu_{ gpu }, device_{ device }, lineWidth{ 1.0f }, frontFace{ VK_FRONT_FACE_CLOCKWISE }, cullMode{ VK_CULL_MODE_NONE }
{
	//initialize input state info
	{
		vertexInput_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput_.pNext = nullptr;
		vertexInput_.flags = 0;
		vertexInput_.vertexBindingDescriptionCount = 0;
		vertexInput_.vertexAttributeDescriptionCount = 0;
	}

	//initialize viewport info
	{
		viewportCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCI_.pNext = nullptr;
		viewportCI_.flags = 0;
	}

	//initialize color blend state
	{
		colorBlendCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendCI_.pNext = nullptr;
		colorBlendCI_.flags = 0;
	}

	//initialize dynamic state
	{
		dynamicStateCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI_.pNext = nullptr;
		dynamicStateCI_.flags = 0;
	}
}

vkn::PipelineBuilder::~PipelineBuilder()
{
}

vkn::Pipeline vkn::PipelineBuilder::get()
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
	//create shaders
	{
		for (const auto& shader : shaders_)
		{
			VkPipelineShaderStageCreateInfo stageInfo{};
			stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageInfo.pNext = nullptr;
			stageInfo.flags = 0;
			stageInfo.stage = shader.stage();
			stageInfo.module = shader.module();
			stageInfo.pName = "main";
			stageInfo.pSpecializationInfo = nullptr;

			shaderStages.push_back(stageInfo);
		}
	}
	std::pair<std::vector<VkVertexInputAttributeDescription>, uint32_t> attributesDesc;
	std::vector<VkVertexInputBindingDescription> bindingsDescriprion;
	auto shader = std::find_if(std::begin(shaders_), std::end(shaders_), [](const auto& shader) { return shader.stage() == VK_SHADER_STAGE_VERTEX_BIT; });
	if (shader != std::end(shaders_))
	{
		if (!std::empty(shader->attributeDescriptions().first) && !std::empty(shader->bindings()))
		{
			attributesDesc = shader->attributeDescriptions();

			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0; //for now, all vertex inputs are binded in slot #0
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			bindingDescription.stride = attributesDesc.second;

			bindingsDescriprion.push_back(bindingDescription);

			vertexInput_.vertexBindingDescriptionCount = std::size(bindingsDescriprion);
			vertexInput_.pVertexBindingDescriptions = std::data(bindingsDescriprion);

			vertexInput_.vertexAttributeDescriptionCount = std::size(attributesDesc.first);
			vertexInput_.pVertexAttributeDescriptions = std::data(attributesDesc.first);
		}
	}
	//set viewport and scissors
	std::vector<VkViewport> viewports;
	std::vector<VkRect2D> scissors;
	auto dynamicViewport = std::find(std::begin(dynamicStates_), std::end(dynamicStates_), VK_DYNAMIC_STATE_VIEWPORT);
	auto dynamicScissor = std::find(std::begin(dynamicStates_), std::end(dynamicStates_), VK_DYNAMIC_STATE_SCISSOR);
	if (dynamicViewport != std::end(dynamicStates_) && dynamicScissor != std::end(dynamicStates_))
	{
		viewports.resize(1);
		scissors.resize(1);
	}
	else
	{
		for (const auto& area : renderAreas_)
		{
			viewports.push_back(area.viewport);
			scissors.push_back(area.scissor);
		}
	}
	viewportCI_.viewportCount = std::size(viewports);
	viewportCI_.pViewports = std::data(viewports);
	viewportCI_.scissorCount = std::size(scissors);
	viewportCI_.pScissors = std::data(scissors);

	//set color blend state attachments
	{
		//if no blend attachments is added by the user, the class automatically
		//create the blend attachements based on the retrospection of the frag shader
		if (std::empty(colorBlendAttachments_))
		{
			auto shader = std::find_if(std::begin(shaders_), std::end(shaders_), [](const auto& shader) { return shader.stage() == VK_SHADER_STAGE_FRAGMENT_BIT; });
			assert(shader != std::end(shaders_) && "Modern pipeline requires  a fragment shader");

			for (auto i = 0u; i < std::size(shader->outputAttachmentsFormats()); ++i)
			{
				VkPipelineColorBlendAttachmentState colorBlendAttachment{};
				colorBlendAttachment.blendEnable = VK_FALSE;
				colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachments_.push_back(colorBlendAttachment);
			}
		}
		colorBlendCI_.attachmentCount = std::size(colorBlendAttachments_);
		colorBlendCI_.pAttachments = std::data(colorBlendAttachments_);
	}

	//set dynamic states
	{
		dynamicStateCI_.dynamicStateCount = std::size(dynamicStates_);
		dynamicStateCI_.pDynamicStates = std::data(dynamicStates_);
	}
	//create the pipeline
	vkn::PipelineLayout pipelineLayout{ device_, shaders_ };
	VkPipeline pipeline{ VK_NULL_HANDLE };
	{
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		{
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = nullptr;
			pipelineInfo.flags = 0;
			pipelineInfo.stageCount = std::size(shaderStages);
			pipelineInfo.pStages = std::data(shaderStages);
			pipelineInfo.pVertexInputState = &vertexInput_;
			pipelineInfo.pInputAssemblyState = &inputAssemblyCI_;
			pipelineInfo.pTessellationState = &tesselationCI_;
			pipelineInfo.pViewportState = &viewportCI_;
			pipelineInfo.pRasterizationState = &rasterizationCI_;
			pipelineInfo.pMultisampleState = &multisampleCI_;
			pipelineInfo.pDepthStencilState = &depthStencilCI_;
			pipelineInfo.pColorBlendState = &colorBlendCI_;
			pipelineInfo.pDynamicState = &dynamicStateCI_;
			pipelineInfo.layout = pipelineLayout.layout;
			pipelineInfo.renderPass = renderpass;
			pipelineInfo.subpass = subpass;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfo.basePipelineIndex = -1;
		}
		vkn::error_check(vkCreateGraphicsPipelines(device_.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline), "Failed to create the pipeline");
	}

	return vkn::Pipeline{ gpu_, device_, pipeline, std::move(shaders_) };
}

void vkn::PipelineBuilder::addShaderStage(const VkShaderStageFlagBits stage, const std::string& path)
{
	auto result = std::find_if(std::begin(shaders_), std::end(shaders_), [&](const auto& shader) { return shader.stage() == stage; });
	if (result != std::end(shaders_))
	{
		throw std::runtime_error{ "There is already a code for this shader stage" };
	}
	shaders_.emplace_back(device_, stage, path);
}

void vkn::PipelineBuilder::addShaderStage(vkn::Shader&& shader)
{
	auto result = std::find_if(std::begin(shaders_), std::end(shaders_), [&](const auto& s) { return s.stage() == shader.stage(); });
	if (result != std::end(shaders_))
	{
		throw std::runtime_error{ "There is already a code for this shader stage" };
	}
	shaders_.emplace_back(std::move(shader));
}

void vkn::PipelineBuilder::setInputBindingRate(const uint32_t binding, const VkVertexInputRate rate)
{
	bool found{ false };
	for (auto i = 0; i < vertexInput_.vertexBindingDescriptionCount; ++i)
	{
		if (vertexInput_.pVertexBindingDescriptions[i].binding == binding)
		{
			found = true;
			const_cast<VkVertexInputBindingDescription&>(vertexInput_.pVertexBindingDescriptions[binding]).inputRate = rate;
		}
	}
	if (!found)
	{
		throw std::runtime_error{ "The binding doesn t match any binding in this pipeline" };
	}
}

void vkn::PipelineBuilder::addAssemblyStage(const VkPrimitiveTopology topology, const VkBool32 restart)
{
	inputAssemblyCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCI_.pNext = nullptr;
	inputAssemblyCI_.flags = 0;
	inputAssemblyCI_.topology = topology;
	inputAssemblyCI_.primitiveRestartEnable = restart;
}

void vkn::PipelineBuilder::addTesselationStage(const uint32_t controlPoints)
{
	tesselationCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tesselationCI_.pNext = nullptr;
	tesselationCI_.flags = 0;
	tesselationCI_.patchControlPoints = controlPoints;
}

void vkn::PipelineBuilder::addViewport(const RenderArea& area)
{
	renderAreas_.push_back(area);
}

void vkn::PipelineBuilder::addRaterizationStage(const VkPolygonMode mode)
{
	rasterizationCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCI_.pNext = nullptr;
	rasterizationCI_.flags = 0;
	rasterizationCI_.depthClampEnable = VK_FALSE;
	rasterizationCI_.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCI_.polygonMode = mode;
	rasterizationCI_.cullMode = cullMode;
	rasterizationCI_.frontFace = frontFace;
	rasterizationCI_.depthBiasEnable = VK_FALSE;
	rasterizationCI_.lineWidth = lineWidth;
}

void vkn::PipelineBuilder::addMultisampleStage(const VkSampleCountFlagBits sampleCount)
{
	multisampleCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCI_.pNext = nullptr;
	multisampleCI_.flags = 0;
	multisampleCI_.rasterizationSamples = sampleCount;
	multisampleCI_.alphaToCoverageEnable = VK_FALSE;
	multisampleCI_.alphaToOneEnable = VK_FALSE;
	multisampleCI_.sampleShadingEnable = VK_FALSE;
}

void vkn::PipelineBuilder::addDepthStage(const VkCompareOp op, const VkBool32 write)
{
	depthStencilCI_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCI_.pNext = nullptr;
	depthStencilCI_.flags = 0;
	depthStencilCI_.depthTestEnable = VK_TRUE;
	depthStencilCI_.depthCompareOp = op;
	depthStencilCI_.depthWriteEnable = write;
	depthStencilCI_.stencilTestEnable = VK_FALSE;
}

void vkn::PipelineBuilder::addStencilStage(const VkStencilOpState front, const VkStencilOpState back)
{
	depthStencilCI_.stencilTestEnable = VK_TRUE;
	depthStencilCI_.front = front;
	depthStencilCI_.back = back;
}

void vkn::PipelineBuilder::addColorBlendStage(const VkBool32 logicEnabled, const VkLogicOp op)
{
	colorBlendCI_.logicOpEnable = logicEnabled;
	colorBlendCI_.logicOp = op;
}

void vkn::PipelineBuilder::addColorBlendAttachment(const VkPipelineColorBlendAttachmentState& attachment)
{
	colorBlendAttachments_.push_back(attachment);
}

void vkn::PipelineBuilder::addDynamicState(const VkDynamicState state)
{
	dynamicStates_.push_back(state);
}

const std::vector<VkFormat>& vkn::PipelineBuilder::getColorOutputFormats() const
{
	auto result = std::find_if(std::begin(shaders_), std::end(shaders_), [](const auto& shader) { return shader.stage() == VK_SHADER_STAGE_FRAGMENT_BIT; });
	if (result == std::end(shaders_))
	{
		return {};
	}
	return result->outputAttachmentsFormats();
}

vkn::PipelineBuilder vkn::PipelineBuilder::getDefault3DPipeline(vkn::Gpu& gpu, vkn::Device& device, const std::string& vertexPath, const std::string& fragmentPath)
{
	vkn::PipelineBuilder builder{gpu, device};
	builder.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexPath);
	builder.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentPath);
	builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder.addDepthStage(VK_COMPARE_OP_LESS);
	builder.addColorBlendStage();
	builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	return std::move(builder);
}
