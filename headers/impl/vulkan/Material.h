#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../camera.h"
#include "../../enums.h"
#include "../../Optics.h"
#include "../../ResourceHolder.h"

#include "vulkanContext.h"
#include "PipelineBuilder.h"
#include "Pipeline.h"
#include "Image.h"
#include "CommandBuffer.h"
#include "meshMemoryLocation.h"
#include "textureImageFactory.h"

namespace gee
{
	class MaterialInstance;
	using MaterialInstanceRef = std::reference_wrapper<MaterialInstance>;
	namespace vkn
	{
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
			void bind(const VkRenderPass& renderpass);
			void set_sampler(const VkSamplerCreateInfo& samplerInfo);
			void use_light(const gee::PointLight& light);
			RENDERPASS_USAGE pass_usage() const;
			const size_t hash() const;
			static const uint32_t max_object_per_instance();
		private:
			Context& context_;
			size_t hash_{};
			const RENDERPASS_USAGE passUsage_;
			vkn::PipelineBuilder builder_;
			std::unique_ptr<Pipeline> pipeline_;
			static uint32_t dynamicAlignment_;
			virtual void prepare_pipeline(Context& context, const RENDERPASS_USAGE& passUsage);
			VkSampler sampler_{ VK_NULL_HANDLE };
			std::unordered_map<gee::TEXTURE_SLOT, std::vector<VkImageView>> textureSlots_;
			std::vector<glm::mat4> transformMatrices_;

			std::vector<gee::ShaderPointLight> pointLights_;
			virtual void build_pipeline(const VkRenderPass& renderpass);
			virtual void set_pointLights();
		};
		MAKE_REFERENCE(Material)
	}
}
namespace std
{
	template<>
	struct hash<gee::vkn::MaterialRef>
	{
		size_t operator()(const gee::vkn::MaterialRef& material) const
		{
			return material.get().hash();
		}
	};
	template<>
	struct equal_to<gee::vkn::MaterialRef>
	{
		bool operator()(const gee::vkn::MaterialRef& lhs, const gee::vkn::MaterialRef& rhs) const
		{
			return lhs.get().hash() == rhs.get().hash();
		}
	};
}