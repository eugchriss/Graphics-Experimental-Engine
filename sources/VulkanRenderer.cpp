#include <string>
#include "../headers/VulkanRenderer.h"
#include "../headers/vulkan_utils.h"
#include <iostream>

#define ENABLE_VALIDATION_LAYERS
vkn::Renderer::Renderer(vkn::Context& _context, const gee::Window& window) : context_{ _context }, commandPool_{ context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT }
{
	swapchain_ = std::make_unique<vkn::Swapchain>(context_);
	createSampler();

	texturesCache_ = std::make_unique<gee::ResourceHolder<vkn::TextureImageFactory, vkn::Image, std::string>>(vkn::TextureImageFactory{ context_ });
	geometriesCache_ = std::make_unique<gee::ResourceHolder<vkn::MeshMemoryLocationFactory, vkn::MeshMemoryLocation, size_t>>(vkn::MeshMemoryLocationFactory{ context_ });

	auto cbs = commandPool_.getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, CB_ALLOCATION_COUNT);
	for (auto& cb : cbs)
	{
		availableCbs_.emplace(std::move(cb));
	}
}

vkn::Renderer::~Renderer()
{
	vkDestroySampler(context_.device->device, sampler_, nullptr);
}

void vkn::Renderer::draw(const gee::Mesh& mesh, const size_t count)
{
	if (shouldRender_)
	{
		auto& cb = availableCbs_.front();
		assert(cb.isRecording() && "Command buffer needs to be in recording state");
		boundPipeline_->get().updateUniforms(cb);
		VkDeviceSize offset{ 0 };
		auto& memoryLocation = geometriesCache_->get(mesh.hash(), mesh);

		vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &memoryLocation.vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(cb.commandBuffer(), memoryLocation.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cb.commandBuffer(), memoryLocation.indicesCount, count, 0, 0, firstInstance_);
		firstInstance_ += count;
	}
}

vkn::CommandBuffer& vkn::Renderer::currentCmdBuffer()
{
	return availableCbs_.front();
}

void vkn::Renderer::begin()
{
	if (std::empty(availableCbs_))
	{
		auto cbs = commandPool_.getCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, CB_ALLOCATION_COUNT);
		for (auto& cb : cbs)
		{
			availableCbs_.emplace(std::move(cb));
		}
	}
	availableCbs_.front().begin();
	shouldRender_ = true;
}

void vkn::Renderer::end()
{
	shouldRender_ = false;
	availableCbs_.front().end();

	if (shouldPresent_)
	{
		context_.graphicsQueue->submitWithFeedbackSignal(availableCbs_.front());
		context_.graphicsQueue->present(*swapchain_, availableCbs_.front().completeSignal());
	}
	else
	{
		context_.graphicsQueue->submit(availableCbs_.front());
	}

	pendingCbs_.emplace_back(std::move(availableCbs_.front()));
	availableCbs_.pop();
	for (auto i = 0u; i < std::size(pendingCbs_); ++i)
	{
		if (pendingCbs_[i].isComplete())
		{
			pendingCbs_[i].completeSignal().reset();
			availableCbs_.emplace(std::move(pendingCbs_[i]));
			pendingCbs_.erase(std::begin(pendingCbs_) + i);
		}
	}
	shouldPresent_ = false;
}

void vkn::Renderer::beginTarget(RenderTarget& target, const VkRect2D& renderArea)
{
	if (!target.isOffscreen())
	{
		shouldRender_ &= target.isReady(*swapchain_);
		shouldPresent_ = true;
	}
	if (shouldRender_)
	{
		auto& cb = availableCbs_.front();
		assert(cb.isRecording() && "Command buffer needs to be in recording state");
		target.bind(cb, renderArea);
		VkViewport viewport{};
		viewport.x = renderArea.offset.x;
		viewport.y = renderArea.offset.y;
		viewport.width = renderArea.extent.width;
		viewport.height = renderArea.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(cb.commandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(cb.commandBuffer(), 0, 1, &renderArea);
	}
}

void vkn::Renderer::endTarget(RenderTarget& target)
{
	if (shouldRender_)
	{
		auto& cb = availableCbs_.front();
		assert(cb.isRecording() && "Command buffer needs to be in recording state");
		target.unBind(cb);
		boundPipeline_.reset();
		isFirstPass_ = true;
	}
}

void vkn::Renderer::clearDepthAttachment(RenderTarget& target, const float clearColor)
{
	if (shouldRender_)
	{
		auto& cb = availableCbs_.front();
		assert(cb.isRecording() && "Command buffer needs to be in recording state");
		target.clearDepthAttachment(cb, clearColor);
	}
}

void vkn::Renderer::usePipeline(Pipeline& pipeline)
{
	if (shouldRender_)
	{
		auto& cb = availableCbs_.front();
		assert(cb.isRecording() && "Command buffer needs to be in recording state");
		
		if (isFirstPass_)
		{
			isFirstPass_ = false;
		}
		else
		{
			vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
		}
		pipeline.bind(cb);
		boundPipeline_ = std::make_optional<std::reference_wrapper<vkn::Pipeline>>(pipeline);
		firstInstance_ = 0;
	}
}

void vkn::Renderer::setTexture(const std::string& name, vkn::Image& image, const VkImageViewType& viewType, const uint32_t layerCount)
{
	if (shouldRender_)
	{
		assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
		boundPipeline_->get().updateTexture(name, sampler_, image.getView(VK_IMAGE_ASPECT_COLOR_BIT, viewType, layerCount), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

void vkn::Renderer::setTexture(const std::string& name, const gee::Texture& texture, const VkImageViewType& viewType, const uint32_t layerCount)
{
	if (shouldRender_)
	{
		assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
		auto& image = texturesCache_->get(texture.paths_[0], texture);
		boundPipeline_->get().updateTexture(name, sampler_, image.getView(VK_IMAGE_ASPECT_COLOR_BIT, viewType, layerCount), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

void vkn::Renderer::setTextures(const std::string& name, const std::vector<std::reference_wrapper<const gee::Texture>>& textures, const VkImageViewType& viewType)
{
	if (shouldRender_)
	{
		assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
		std::vector<VkImageView> views;
		for (const auto& texture : textures)
		{
			views.emplace_back(texturesCache_->get(texture.get().paths_[0], texture).getView(VK_IMAGE_ASPECT_COLOR_BIT, viewType));
		}
		boundPipeline_->get().updateTextures(name, sampler_, views, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

void vkn::Renderer::beginQuery(Query& query)
{
	auto& cb = availableCbs_.front();
	assert(cb.isRecording() && "Command buffer needs to be in recording state");
	query.begin(cb);
}

void vkn::Renderer::endQuery(Query& query)
{
	auto& cb = availableCbs_.front();
	assert(cb.isRecording() && "Command buffer needs to be in recording state");
	query.end(cb);
}

vkn::Query vkn::Renderer::writeTimestamp(QueryPool& queryPool, const VkPipelineStageFlagBits stage)
{
	auto& cb = availableCbs_.front();
	assert(cb.isRecording() && "Command buffer needs to be in recording state");
	auto query = queryPool.getQuery(cb);
	query.writeTimeStamp(cb, stage);
	return query;
}

vkn::Swapchain& vkn::Renderer::swapchain()
{
	return *swapchain_;
}

void vkn::Renderer::createSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	vkn::error_check(vkCreateSampler(context_.device->device, &samplerInfo, nullptr, &sampler_), "Failed to create the sampler");
}
