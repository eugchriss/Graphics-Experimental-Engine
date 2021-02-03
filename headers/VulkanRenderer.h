#pragma once
#include <optional>
#include <string>
#include <memory>
#include <queue>
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "vulkanContext.h"
#include "Window.h"
#include "Swapchain.h"
#include "Queue.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Drawable.h"
#include "Material.h"
#include "Camera.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "ResourceHolder.h"
#include "textureImageFactory.h"
#include "meshMemoryLocation.h"
#include "Query.h"
#include "QueryPool.h"

namespace vkn
{
	class Renderer
	{
	public:
		Renderer(vkn::Context& _context, const gee::Window& window);
		~Renderer();
		void draw(const gee::Mesh& mesh, const size_t count = 1);
		CommandBuffer& currentCmdBuffer();
		void begin();
		void end();
		void beginTarget(RenderTarget& target, const VkRect2D& renderArea);
		void endTarget(RenderTarget& target);
		void clearDepthAttachment(RenderTarget& target, const float clearColor = 1.0f);
		void usePipeline(Pipeline& pipeline);
		void setTexture(const std::string& name, const gee::Texture& texture, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t layerCount = 1);
		void setTextures(const std::string& name, const std::vector<std::reference_wrapper<const gee::Texture>>& textures, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D);
		template <class T>
		void updateBuffer(const std::string& name, const T& datas);
		template <class T>
		void updateSmallBuffer(const std::string& name, const T& datas);
		void beginQuery(Query& query);
		void endQuery(Query& query);
		Query writeTimestamp(QueryPool& queryPool, const VkPipelineStageFlagBits stage);
		Swapchain& swapchain();

	private:
		const uint32_t CB_ALLOCATION_COUNT = 10u;
		vkn::Context& context_;
		std::unique_ptr<Swapchain> swapchain_;
		VkSampler sampler_{ VK_NULL_HANDLE };
		CommandPool commandPool_;
		std::queue<vkn::CommandBuffer> availableCbs_;
		std::vector<vkn::CommandBuffer> pendingCbs_;
		bool isWindowMinimized_{};
		bool shouldRender_{ false };
		bool shouldPresent_{ false };
		uint32_t firstInstance_{};
		template <class T>
		using OptionalRef = std::optional<std::reference_wrapper<T>>;

		OptionalRef<Pipeline> boundPipeline_;
		std::vector<OptionalRef<Pipeline>> boundPipelines_;

		template <class Factory, class Resource, class Key>
		using CachePtr = std::unique_ptr<gee::ResourceHolder<Factory, Resource, Key>>;

		CachePtr<vkn::TextureImageFactory, vkn::Image, std::string> texturesCache_;
		CachePtr<vkn::MeshMemoryLocationFactory, vkn::MeshMemoryLocation, size_t> geometriesCache_;
		std::vector<VkImageView> pipelineTextureViews_;
		void createSampler();
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
			assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
			boundPipeline_->get().pushConstant(availableCbs_.front(), name, datas);
		}
	}
}