#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "geometry.h"
#include "Renderpass.h"
#include "ResourceHolder.h"
#include "Texture.h"
#include "VulkanContext.h"
#include "Window.h"

#include "impl/vulkan/commandPool.h"
#include "impl/vulkan/meshMemoryLocation.h"
#include "impl/vulkan/Pipeline.h"
#include "impl/vulkan/PipelineBuilder.h"
#include "impl/vulkan/Renderpass.h"
#include "impl/vulkan/Sampler.h"
#include "impl/vulkan/Swapchain.h"
#include "impl/vulkan/vulkanContext.h"

namespace gee
{
	class Renderer
	{
	public:
		Renderer(gee::VulkanContext& context, gee::Window& window);
		~Renderer();
		void start_renderpass(const Renderpass& rp);
		void use_shader_technique(const ShaderTechnique& technique);
		void new_batch();
		void render_gui();
		void draw(const Geometry& geometry);
		void update_shader_value(const ShaderValue& val);
		void update_shader_value(const ShaderTexture& texture);
		void update_shader_value(const ShaderArrayTexture& arrayTexture);
		void push_shader_constant(const ShaderValue& val);
		void render();
	private:
		bool renderGui_{ false };
		VkRect2D windowExtent_;
		gee::VulkanContext& context_;
		std::unique_ptr<vkn::Swapchain> swapchain_;
		std::unique_ptr<vkn::CommandPool> cmdPool_;
		ID<Renderpass>::Type currentRenderpass_;
		ID<ShaderTechnique>::Type currentShaderTechnique_;
		size_t currentBatchIndex_{};
		ResourceHolder<vkn::Pipeline, ID<ShaderTechnique>::Type> shaderTechniques_;
		ResourceHolder<vkn::Renderpass, ID<Renderpass>::Type> renderpasses_;
		ResourceHolder<vkn::RenderTarget, ID<RenderTarget>::Type> renderTargets_;
		ResourceHolder<vkn::GeometryMemoryLocation, ID<GeometryConstRef>::Type> geometryMemories_;
		ResourceHolder<vkn::Image, ID<Texture>::Type> textures_;
		ResourceHolder<vkn::Sampler, ID<Sampler>::Type> samplers_;
		std::unordered_map<ID<Renderpass>::Type, std::vector<ID<ShaderTechnique>::Type>> drawList_;
		std::unordered_map<ID<ShaderTechnique>::Type, std::vector<std::vector<std::pair<GeometryConstRef, uint32_t>>>> shaderTechniqueGeometriesBatchs_;
		std::unordered_map<ID<ShaderTechnique>::Type, std::vector<ShaderValue>> shaderTechniqueValues_;
		std::unordered_map<ID<ShaderTechnique>::Type, std::vector<vkn::ShaderConstant>> shaderTechniqueConstants_;
		std::unordered_map<ID<ShaderTechnique>::Type, std::vector<ShaderTexture>> shaderTechniqueTextures_;
		std::unordered_map<ID<ShaderTechnique>::Type, std::vector<std::reference_wrapper<const ShaderArrayTexture>>> shaderTechniqueArrayTextures_;

		Observer_ptr<vkn::CommandBuffer> previousCmdBuffer_{};
	};
}
