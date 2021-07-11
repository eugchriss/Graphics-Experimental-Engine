#include "..\headers\Renderer.h"

#include "../headers/impl/vulkan/vulkanContextBuilder.h"
gee::Renderer::Renderer(const std::string& windowTitle, const uint32_t width, const uint32_t height) : window_{ windowTitle, width, height }
{
	windowExtent_ = VkRect2D{ .offset = {.x = 0, .y = 0},
							  .extent = {.width = width, .height = height} };
	create_context();
	cmdPool_ = std::make_unique<vkn::CommandPool>(*context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	swapchain_ = std::make_unique<vkn::Swapchain>(*context_);
}

gee::Renderer::~Renderer()
{
	context_->device->idle();
}

void gee::Renderer::start_renderpass(const Renderpass& renderpass)
{
	auto rp = ID<Renderpass>::get(renderpass);
	renderpasses_.get(rp, *context_, renderTargets_, *swapchain_, renderpass);
	drawList_[rp] = {};
	currentRenderpass_ = rp;
}

void gee::Renderer::use_shader_technique(const ShaderTechnique& technique)
{
	currentShaderTechnique_ = ID<ShaderTechnique>::get(technique);
	drawList_[currentRenderpass_].emplace_back(currentShaderTechnique_);

	shaderTechniques_.get(currentShaderTechnique_, *context_, renderpasses_.get(currentRenderpass_), technique);
	shaderTechniqueGeometriesBatchs_[currentShaderTechnique_] = {};
	shaderTechniqueValues_[currentShaderTechnique_] = {};
	shaderTechniqueTextures_[currentShaderTechnique_] = {};
	shaderTechniqueConstants_[currentShaderTechnique_] = {};
	shaderTechniqueArrayTextures_[currentShaderTechnique_] = {};
}

void gee::Renderer::new_batch()
{
	currentBatchIndex_ = 0;
	auto result = shaderTechniqueGeometriesBatchs_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueGeometriesBatchs_) && "No shader technique");
	shaderTechniqueGeometriesBatchs_[currentShaderTechnique_].emplace_back();
}

void gee::Renderer::draw(const Geometry& geometry)
{
	auto geometryID = ID<GeometryConstRef>::get(std::cref(geometry));
	auto result = shaderTechniqueGeometriesBatchs_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueGeometriesBatchs_) && "No shader technique");
	auto& batches = shaderTechniqueGeometriesBatchs_[currentShaderTechnique_];
	assert(!std::empty(batches) && "need to call new_batch() first");

	auto& currentBatch = batches.back();
	auto alreadyExist = std::find_if(std::begin(currentBatch), std::end(currentBatch), [&](auto& pair) {return ID<GeometryConstRef>::get(pair.first) == geometryID; });
	if (alreadyExist != std::end(currentBatch))
	{
		alreadyExist->second++;
	}
	else
	{
		currentBatch.emplace_back(geometry, 1);
	}
}

void gee::Renderer::update_shader_value(const ShaderValue& val)
{
	auto result = shaderTechniqueValues_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueValues_) && "No shader technique");
	result->second.emplace_back(val);
}

void gee::Renderer::push_shader_constant(const ShaderValue& val)
{
	auto result = shaderTechniqueConstants_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueConstants_) && "No shader technique");
	result->second.emplace_back(vkn::ShaderConstant{ .value = val, .batchIndex = std::size(shaderTechniqueGeometriesBatchs_[currentShaderTechnique_]) - 1 });
}

void gee::Renderer::update_shader_value(const ShaderTexture& texture)
{
	auto result = shaderTechniqueTextures_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueTextures_) && "No shader technique");
	result->second.emplace_back(texture);
}

void gee::Renderer::update_shader_value(const ShaderArrayTexture& arrayTexture)
{
	auto result = shaderTechniqueArrayTextures_.find(currentShaderTechnique_);
	assert(result != std::end(shaderTechniqueArrayTextures_) && "No shader technique");
	result->second.emplace_back(arrayTexture);
}

void gee::Renderer::resize()
{
}

bool gee::Renderer::render()
{
	if (window_.isVisible() && (!previousCmdBuffer_ || !previousCmdBuffer_->isPending()))
	{
		for (const auto& [pipelineID, shaderValues] : shaderTechniqueValues_)
		{
			auto& pipeline = shaderTechniques_.get(pipelineID);
			for (auto& shaderValue : shaderValues)
			{
				pipeline.update_shader_value(shaderValue);
			}
		}
		for (const auto& [pipelineID, shaderTextures] : shaderTechniqueTextures_)
		{
			auto& pipeline = shaderTechniques_.get(pipelineID);
			for (auto& texture : shaderTextures)
			{
				vkn::ShaderTexture image{ .name = texture.name };
				image.view = textures_.get(ID<Texture>::get(texture.texture), *context_, *cmdPool_, texture.texture).getView(VK_IMAGE_ASPECT_COLOR_BIT);
				image.sampler = samplers_.get(ID<Sampler>::get(texture.sampler), *context_, texture.sampler).sampler;
				pipeline.update_shader_texture(image);
			}
		}
		for (const auto& [pipelineID, shaderArrayTextures] : shaderTechniqueArrayTextures_)
		{
			auto& pipeline = shaderTechniques_.get(pipelineID);
			for (auto& arrayTextureRef : shaderArrayTextures)
			{
				auto& arrayTexture = arrayTextureRef.get();
				vkn::ShaderArrayTexture images{ .name = arrayTexture.name };
				for (const auto texture : arrayTexture.textures)
				{
					if (texture.has_value())
					{
						images.views.emplace_back(textures_.get(ID<Texture>::get(texture->get()), *context_, *cmdPool_, texture->get()).getView(VK_IMAGE_ASPECT_COLOR_BIT));
					}
					else
					{
						images.views.emplace_back(VkImageView{ VK_NULL_HANDLE });
					}
				}
				images.sampler = samplers_.get(ID<Sampler>::get(arrayTexture.sampler), *context_, arrayTexture.sampler).sampler;
				pipeline.update_shader_array_texture(images);
			}
		}
		auto& cb = cmdPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		cb.begin();
		VkDeviceSize offset{ 0 };
		for (const auto& [renderpassID, shaderTechniques] : drawList_)
		{
			auto& rp = renderpasses_.get(renderpassID);
			rp.begin(cb, windowExtent_);
			for (const auto& pipelineID : shaderTechniques)
			{
				auto& pipeline = shaderTechniques_.get(pipelineID);
				pipeline.updateUniforms();
				pipeline.bind(cb);

				for (auto i = 0u; i < std::size(shaderTechniqueGeometriesBatchs_[pipelineID]); ++i)
				{
					vkn::Pipeline::DescriptorSetOffsets boundSet{ .set = 0 };
					boundSet.offsets.emplace_back(i);
					std::vector<vkn::Pipeline::DescriptorSetOffsets> v{ boundSet };
					pipeline.bind_set(cb, v);
					const auto& pushConstants = shaderTechniqueConstants_[pipelineID];
					for (const auto& pushConstant : pushConstants)
					{
						if (pushConstant.batchIndex == i)
						{
							pipeline.push_constant(cb, pushConstant.value);
						}
					}
					for (auto& [geometryRef, occurenceCount]  : shaderTechniqueGeometriesBatchs_[pipelineID][i])
					{
						auto geometryID = ID<gee::GeometryConstRef>::get(geometryRef);
						auto& geometry = geometryMemories_.get(geometryID, *context_, geometryRef);

						vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &geometry.vertexBuffer.buffer, &offset);
						vkCmdBindIndexBuffer(cb.commandBuffer(), geometry.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

						vkCmdDrawIndexed(cb.commandBuffer(), geometry.indicesCount, occurenceCount, 0, 0, 0);
					}
				}
			}
			rp.end(cb);
		}
		cb.end();
		swapchain_->swapBuffers();
		context_->graphicsQueue->submit(cb);
		context_->graphicsQueue->present(cb, *swapchain_);

		drawList_.clear();
		shaderTechniqueGeometriesBatchs_.clear();
		previousCmdBuffer_.reset(&cb);
	}
	return window_.isOpen();
}

GLFWwindow* gee::Renderer::window_handle()
{
	return window_.window();
}

const glm::u32vec2 gee::Renderer::window_size() const
{
	return window_.size();
}

float gee::Renderer::aspect_ratio() const
{
	return window_.aspectRatio();
}

void gee::Renderer::create_context()
{
	uint32_t glfwExtensionCount = 0;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	vkn::ContextBuilder contextBuilder{ severityFlags, messageTypeFlags };
	contextBuilder.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	contextBuilder.addInstanceLayer("VK_LAYER_LUNARG_monitor");
	for (const auto& instanceExtension : instanceExtensions)
	{
		contextBuilder.addInstanceExtention(instanceExtension);
	}
	contextBuilder.addDeviceExtention("VK_KHR_swapchain");
	contextBuilder.addQueueFlag(VK_QUEUE_GRAPHICS_BIT);
	contextBuilder.addQueueFlag(VK_QUEUE_TRANSFER_BIT);
	contextBuilder.setQueueCount(2);

	context_ = std::make_unique<vkn::Context>(std::move(contextBuilder.build(window_)));
}
