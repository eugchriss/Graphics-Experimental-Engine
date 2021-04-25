#pragma once
#include "ResourceHolder.h"
#include "vulkanContext.h"
#include "PipelineBuilder.h"
#include "Pipeline.h"
#include "Image.h"
#include "CommandBuffer.h"
#include "meshMemoryLocation.h"
#include "textureImageFactory.h"
#include "camera.h"
#include "Optics.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

enum class TEXTURE_SLOT
{
	COLOR,
	NORMAL,
	SPECULAR
};

namespace gee
{
	class MaterialInstance;
}

namespace vkn
{
	enum RENDERPASS_USAGE
	{
		COLOR_PASS = 0x0,
		SKYBOX_PASS = 0x1,
		UI_PASS = 0x02
	};
	using GeometryMemoryHolder = gee::ResourceHolder<vkn::GeometryMemoryLocationFactory, vkn::GeometryMemoryLocation, size_t>;
	using TextureMemoryHolder = gee::ResourceHolder<vkn::TextureImageFactory, vkn::Image>;
	class Material
	{
	public:
		Material(Context& context, const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const RENDERPASS_USAGE& passUsage = COLOR_PASS);
		Material(Material&& other);
		virtual ~Material();
		void bind(const VkRenderPass& renderpass);
		virtual void draw(GeometryMemoryHolder& memoryHolder, TextureMemoryHolder& imageHolder, CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<gee::MaterialInstance>& materialInstances);
		void set_sampler(const VkSamplerCreateInfo& samplerInfo);
		void use_light(const gee::PointLight& light);
		RENDERPASS_USAGE pass_usage() const;
		const size_t hash() const;
	private:
		Context& context_;
		size_t hash_{};
		const RENDERPASS_USAGE passUsage_;
		vkn::PipelineBuilder builder_;
		std::unique_ptr<Pipeline> pipeline_;
		virtual void prepare_pipeline(Context& context, const RENDERPASS_USAGE& passUsage);
		VkSampler sampler_{ VK_NULL_HANDLE };
		std::unordered_map<TEXTURE_SLOT, std::vector<VkImageView>> textureSlots_;
		std::vector<glm::mat4> transformMatrices_;
		void getPackedTextures_and_transforms(TextureMemoryHolder& imageHolder, std::vector<gee::MaterialInstance>&);

		std::vector<gee::ShaderPointLight> pointLights_;
		virtual void build_pipeline(const VkRenderPass& renderpass);
		virtual void set_pointLights();
	};
	MAKE_REFERENCE(Material)	
}
namespace std
{
	template<>
	struct hash<vkn::MaterialRef>
	{
		size_t operator()(const vkn::MaterialRef& material) const
		{
			return material.get().hash();
		}
	};
	template<>
	struct equal_to<vkn::MaterialRef>
	{
		bool operator()(const vkn::MaterialRef& lhs, const vkn::MaterialRef& rhs) const
		{
			return lhs.get().hash() == rhs.get().hash();
		}
	};
}