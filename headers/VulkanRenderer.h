#pragma once
#include <optional>
#include <string>
#include <memory>
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "vulkanContext.h"
#include "Window.h"
#include "Swapchain.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "Drawable.h"
#include "Material.h"
#include "Camera.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "ResourceHolder.h"
#include "textureImageFactory.h"
#include "meshMemoryLocation.h"

namespace vkn
{
	class Renderer
	{
	public:
		Renderer(vkn::Context& _context, const gee::Window& window);
		~Renderer();
		void draw(const gee::Mesh& mesh, const size_t count = 1);
		void begin(RenderTarget& target, const VkRect2D& renderArea);
		void end(RenderTarget& target);
		void usePipeline(Pipeline& pipeline);
		void setTexture(const std::string& name, const gee::Texture& texture, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t layerCount = 1);
		void setTextures(const std::string& name, const std::vector<std::reference_wrapper<const gee::Texture>>& textures, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D);
		template <class T>
		void updateBuffer(const std::string& name, const T& datas);
		template <class T>
		void updateSmallBuffer(const std::string& name, const T& datas);
		Swapchain& swapchain();

	private:
		vkn::Context& context_;
		std::unique_ptr<Swapchain> swapchain_;
		VkSampler sampler_{ VK_NULL_HANDLE };
		bool isWindowMinimized_{};
		bool shouldRender_{ false };
		uint32_t firstInstance_{};
		template <class T>
		using OptionalRef = std::optional<std::reference_wrapper<T>>;

		OptionalRef<CommandBuffer> currentCb_;
		OptionalRef<Pipeline> boundPipeline_;
		std::vector<OptionalRef<Pipeline>> boundPipelines_;

		template <class Factory, class Resource, class Key>
		using CachePtr = std::unique_ptr<gee::ResourceHolder<Factory, Resource, Key>>;

		CachePtr<vkn::TextureImageFactory, vkn::Image, std::string> texturesCache_;
		CachePtr<vkn::MeshMemoryLocationFactory, vkn::MeshMemoryLocation, size_t> geometriesCache_;
		std::vector<VkImageView> pipelineTextureViews_;
		//imgui variables
		void createSampler();
		//void buildImguiContext(const gee::Window& window);
	};
	template<class T>
	inline void Renderer::updateBuffer(const std::string& name, const T& datas)
	{
		if (shouldRender_)
		{
			assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
			boundPipeline_->get().updateBuffer(name, datas);
		}
	}
	template<class T>
	inline void Renderer::updateSmallBuffer(const std::string& name, const T& datas)
	{
		if (shouldRender_)
		{
			assert(currentCb_.has_value() && "The render target need to be bind first");
			assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
			boundPipeline_->get().pushConstant(currentCb_->get(), name, datas);
		}
	}
}