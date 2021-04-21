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
#include <unordered_map>

enum class TEXTURE_SLOT
{
	COLOR,
	NORMAL,
	SPECULAR
};
class gee::Mesh;

struct GeometryInstances
{
	GeometryInstances(vkn::GeometryMemoryLocation& geometryMemory) : geometryMemoryRef{ std::ref(geometryMemory) } {}
	GeometryInstances(GeometryInstances&& other) = default;
	vkn::GeometryMemoryLocationRef geometryMemoryRef;
	std::vector<glm::mat4> transformMatrices;
};

struct MaterialInstance
{
	MaterialInstance() = default;
	MaterialInstance(MaterialInstance&& other) = default;
	void set_property(const TEXTURE_SLOT slot, vkn::Image& image)
	{
		textureSlots[slot] = image.getView(VK_IMAGE_ASPECT_COLOR_BIT);
	}
	void add_geometry(GeometryInstances&& geometry)
	{
		geometries.emplace_back(std::move(geometry));
	}
	std::unordered_map<TEXTURE_SLOT, VkImageView> textureSlots;
	std::vector<GeometryInstances> geometries;
};
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
		virtual void draw(CommandBuffer& cb, const gee::Camera::ShaderInfo& cameraShaderInfo, const std::vector<MaterialInstance>& materialInstances);
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
		std::unordered_map<TEXTURE_SLOT, std::vector<VkImageView>> textureSlots_;
		std::vector<glm::mat4> transformMatrices_;
		void getPackedTextures_and_transforms(std::vector<MaterialInstance>&);
	};
}