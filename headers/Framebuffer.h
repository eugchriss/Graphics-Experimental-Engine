#pragma once
#include <unordered_map>
#include <set>
#include "vulkan/vulkan.hpp"
#include "gpu.h"
#include "Device.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "ShaderEffect.h"
#include "image.h"
#include "queue.h"
#include "CommandPool.h"
#include "../headers/imgui_impl_vulkan.h"

#include <vector>

namespace vkn
{
	using TextureKey_t = std::string;
	using TextureHolder_t = gee::ResourceHolder<vkn::TextureImageFactory, vkn::Image, TextureKey_t>;
	using MaterialKey_t = Hash_t;
	using MaterialHolder_t = gee::ResourceHolder<gee::MaterialHelperFactory<TextureKey_t>, gee::MaterialHelper<TextureKey_t>, MaterialKey_t>;
	using TextureSet_t = std::unordered_set<TextureKey_t>;
	using MaterialSet_t = std::vector<MaterialKey_t>;
	struct ShaderCamera
	{
		glm::vec4 position{};
		glm::mat4 view{};
		glm::mat4 projection{};
	};
	class Framebuffer
	{
	public:

		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, VkSurfaceKHR surface, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const bool enableGui = true, ImGui_ImplVulkan_InitInfo guiInfo = {}, const uint32_t frameCount = 2);
		Framebuffer(vkn::Gpu& gpu, vkn::Device& device, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const VkExtent2D& extent, const uint32_t frameCount = 2);
		Framebuffer(Framebuffer&& other) = default;
		~Framebuffer();
#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		void resize(const glm::u32vec2& size);
		const std::vector<vkn::Pixel> frameContent(const uint32_t index);
		const float rawContentAt(const uint32_t index, const VkDeviceSize offset);
		const std::vector<float> frameRawContent(const uint32_t index);
		void setupRendering(const std::string& effectName, const vkn::ShaderCamera& camera, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		void setupRendering(const std::string& effectName, const vkn::ShaderCamera& camera, const std::reference_wrapper<gee::Drawable>& drawable);
		void render(MeshHolder_t& meshHolder, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const VkSampler& sampler);
		void submitTo(vkn::Queue& graphicsQueue);
		void setViewport(const float x, const float y, const float width, const float height);
		vkn::ShaderEffect& getEffect(const std::string& name);
	private:
		vkn::Gpu& gpu_;
		vkn::Device& device_;
		vkn::CommandPool& cbPool_;
		std::unique_ptr<vkn::Swapchain> swapchain_;
		std::vector<VkFramebuffer> framebuffers_;
		Ptr<vkn::Renderpass> renderpass_;
		std::vector<vkn::Image> images_;
		std::vector<vkn::CommandBuffer> cbs_;
		std::vector<vkn::Signal> imageAvailableSignals_;
		std::vector<vkn::Signal> renderingFinishedSignals_;
		vkn::Shader::Attachment finalColorAttachment_;
		uint32_t currentFrame_{};
		VkRect2D renderArea_{};
		bool guiEnabled_{ false };
		uint32_t outputColorBufferAttachmentIndex{};
		using DrawablesRef = std::reference_wrapper<const std::vector<std::reference_wrapper<gee::Drawable>>>;
		using ShaderEffectRef = std::reference_wrapper<vkn::ShaderEffect>;
		using CameraConstRef = std::reference_wrapper<const vkn::ShaderCamera>;
		std::unordered_map<std::string, vkn::ShaderEffect> effects_;
		std::vector<std::tuple<ShaderEffectRef, DrawablesRef, CameraConstRef>> renderingOrder_;
		VkImageAspectFlags getAspectFlag(const vkn::RenderpassAttachment& attachment);
		VkImageUsageFlags getUsageFlag(const vkn::RenderpassAttachment& attachment);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<vkn::RenderpassAttachment>& renderpassAttachments, const uint32_t frameCount);
		void createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<vkn::RenderpassAttachment>& renderpassAttachments, const uint32_t presentAttachmentIndex, const uint32_t frameCount);
		void createSignals();
		bool shouldRender();
		void createRenderpass(const std::unordered_map<std::string, vkn::ShaderEffect>& effectRefs, const bool enableGui);
		const std::vector<RenderpassAttachment> createRenderpassAttachments(vkn::RenderpassBuilder& builder, const std::unordered_map<std::string, vkn::ShaderEffect>& effectRefs);
		void createRenderpassSubpasses(vkn::RenderpassBuilder& builder, const std::unordered_map<std::string, vkn::ShaderEffect>& effectRefs, const std::vector<RenderpassAttachment>& attachments);
		void createSubpassesDepencies(vkn::RenderpassBuilder& builder, const std::unordered_map<std::string, vkn::ShaderEffect>& effectRefs, const std::vector<RenderpassAttachment>& attachments);
		void initGui(vkn::CommandBuffer& cb, ImGui_ImplVulkan_InitInfo& info);
		using ShaderEffectResource = std::tuple<gee::Occurence<Hash_t>, TextureSet_t, MaterialSet_t, std::vector<MaterialKey_t>, std::vector<glm::mat4>>;
		const ShaderEffectResource createShaderEffectResource(MeshHolder_t& meshHolder, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		void bindTexture(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const;
		void bindSkybox(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const;
		void bindMaterial( MaterialHolder_t& materialHolder, const TextureSet_t textures, const MaterialSet_t& materials, vkn::ShaderEffect& effect) const;
		void bindDrawableMaterial(MaterialHolder_t& materialHolder, const std::vector<Hash_t>& materials, vkn::ShaderEffect& effect) const;
		void bindUniforms(vkn::ShaderEffect& effect, const vkn::ShaderCamera& camera, VkSampler sampler, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const TextureSet_t& textures, const MaterialSet_t& materials, const std::vector<MaterialKey_t>& drawableMaterials, const std::vector<glm::mat4>& transforms);
		const bool isLastEffect(const vkn::ShaderEffect& effect) const;
		void createGuiSubpass(vkn::RenderpassBuilder& builder, const std::vector<RenderpassAttachment>& attachments);
		template<class T>
		size_t indexOf(const std::unordered_set<T>& set, const T& value) const;
	};
	template<class T>
	inline size_t Framebuffer::indexOf(const std::unordered_set<T>& set, const T& value) const
	{
		auto result = set.find(value);
		assert(result != set.end() && "Indexing a non existing element");
		return std::distance(set.begin(), result);
	}
}