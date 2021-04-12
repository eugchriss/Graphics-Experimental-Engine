#pragma once
#include "vulkanContext.h"
#include "PipelineBuilder.h"
#include "Pipeline.h"
#include "Image.h"
#include "Mesh.h"
#include "CommandBuffer.h"
#include "meshMemoryLocation.h"
#include "camera.h"
#include <string>
#include <memory>
#include <vector>


namespace vkn
{
	struct GeometyInstances
	{
		GeometyInstances(GeometryMemoryLocation& geometryMemory) : geometryMemory{geometryMemory}{}
		GeometyInstances(GeometyInstances&&) = default;
		GeometryMemoryLocation& geometryMemory;
		std::vector<glm::mat4> transformMatrices;
	};
	enum RENDERPASS_USAGE
	{
		COLOR_PASS = 0x0,
		SKYBOX_PASS = 0x1,
		UI_PASS = 0x02
	};
	class Material
	{
	public:
		Material(Context& context, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const RENDERPASS_USAGE& passUsage = COLOR_PASS);
		Material(Material&& other);
		virtual ~Material();
		void set_base_color(vkn::Image& image);
		void bind(const VkRenderPass& renderpass);
		virtual void draw(CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<GeometyInstances>& geometryInstances);
		void set_sampler(const VkSamplerCreateInfo& samplerInfo);
		RENDERPASS_USAGE pass_usage() const;

	protected:
		virtual void build_pipeline(const VkRenderPass& renderpass);
	private:
		Context& context_;
		const RENDERPASS_USAGE passUsage_;
		vkn::PipelineBuilder builder_;
		std::unique_ptr<Pipeline> pipeline_;
		virtual void prepare_pipeline(Context& context, const RENDERPASS_USAGE& passUsage);
		VkSampler sampler_{ VK_NULL_HANDLE };
		VkImageView colorView_{ VK_NULL_HANDLE };
		std::vector<glm::mat4> transformMatrices_;
	};
}