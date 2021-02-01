#include <string>
#include "../headers/VulkanRenderer.h"
#include "../headers/vulkan_utils.h"

#define ENABLE_VALIDATION_LAYERS
vkn::Renderer::Renderer(vkn::Context& _context, const gee::Window& window) : context_{ _context }
{
	swapchain_ = std::make_unique<vkn::Swapchain>(context_);

	createSampler();

	texturesCache_ = std::make_unique<gee::ResourceHolder<vkn::TextureImageFactory, vkn::Image, std::string>>(vkn::TextureImageFactory{ context_ });
	geometriesCache_ = std::make_unique<gee::ResourceHolder<vkn::MeshMemoryLocationFactory, vkn::MeshMemoryLocation, size_t>>(vkn::MeshMemoryLocationFactory{ context_ });
	//buildImguiContext(window);
}

vkn::Renderer::~Renderer()
{
	vkDestroySampler(context_.device->device, sampler_, nullptr);
}

void vkn::Renderer::draw(const gee::Mesh& mesh, const size_t count)
{
	if (shouldRender_)
	{
		assert(currentCb_.has_value() && "The render target need to be bind first");
		VkDeviceSize offset{ 0 };
		auto& memoryLocation = geometriesCache_->get(mesh.hash(), mesh);

		vkCmdBindVertexBuffers(currentCb_->get().commandBuffer(), 0, 1, &memoryLocation.vertexBuffer.buffer, &offset);
		vkCmdBindIndexBuffer(currentCb_->get().commandBuffer(), memoryLocation.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(currentCb_->get().commandBuffer(), memoryLocation.indicesCount, count, 0, 0, firstInstance_);
		firstInstance_ += count;
	}
}

void vkn::Renderer::begin(RenderTarget& target, const VkRect2D& renderArea)
{
	shouldRender_ = target.isReady();
	if (shouldRender_)
	{
		currentCb_ = std::make_optional<std::reference_wrapper<vkn::CommandBuffer>>(target.bind(renderArea));
		VkViewport viewport{};
		viewport.x = renderArea.offset.x;
		viewport.y = renderArea.offset.y;
		viewport.width = renderArea.extent.width;
		viewport.height = renderArea.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(currentCb_->get().commandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(currentCb_->get().commandBuffer(), 0, 1, &renderArea);
	}
}

void vkn::Renderer::end(RenderTarget& target)
{
	if (shouldRender_)
	{
		target.unBind();
		for (auto& pipeline : boundPipelines_)
		{
			pipeline->get().updateUniforms();
		}
		boundPipelines_.clear();
		currentCb_.reset();
		boundPipeline_.reset();
		if (!target.isOffscreen())
		{
			swapchain_->setImageAvailableSignal(target.imageAvailableSignal());
			context_.graphicsQueue->submit(currentCb_->get(), target.renderingFinishedSignal(), target.imageAvailableSignal(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, true);
			context_.graphicsQueue->present(*swapchain_, target.renderingFinishedSignal());
		}
		else
		{
			context_.graphicsQueue->submit(currentCb_->get(), target.renderingFinishedSignal());
		}
	}
}

void vkn::Renderer::usePipeline(Pipeline& pipeline)
{
	if (shouldRender_)
	{
		assert(currentCb_.has_value() && "The render target need to be bind first");
		if (!std::empty(boundPipelines_))
		{
			vkCmdNextSubpass(currentCb_->get().commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
		}
		pipeline.bind(currentCb_.value());
		boundPipeline_ = std::make_optional<std::reference_wrapper<vkn::Pipeline>>(pipeline);
		boundPipelines_.emplace_back(boundPipeline_);
		firstInstance_ = 0;
	}
}

void vkn::Renderer::setTexture(const std::string& name, const gee::Texture& texture, const VkImageViewType& viewType, const uint32_t layerCount)
{
	if (shouldRender_)
	{
		assert(currentCb_.has_value() && "The render target need to be bind first");
		assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
		auto& image = texturesCache_->get(texture.paths_[0], texture);
		boundPipeline_->get().updateTexture(name, sampler_, image.getView(VK_IMAGE_ASPECT_COLOR_BIT, viewType, layerCount), VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

void vkn::Renderer::setTextures(const std::string& name, const std::vector<std::reference_wrapper<const gee::Texture>>& textures, const VkImageViewType& viewType)
{
	if (shouldRender_)
	{
		assert(currentCb_.has_value() && "The render target need to be bind first");
		assert(boundPipeline_.has_value() && "A pipeline needs to bind first");
		std::vector<VkImageView> views;
		for (const auto& texture : textures)
		{
			views.emplace_back(texturesCache_->get(texture.get().paths_[0], texture).getView(VK_IMAGE_ASPECT_COLOR_BIT, viewType));
		}
		boundPipeline_->get().updateTextures(name, sampler_, views, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
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

/*void vkn::Renderer::buildImguiContext(const gee::Window& window)
{

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.maxSets = 1000 * std::size(pool_sizes);
	poolInfo.poolSizeCount = std::size(pool_sizes);
	poolInfo.pPoolSizes = std::data(pool_sizes);
	vkn::error_check(vkCreateDescriptorPool(device_->device, &poolInfo, nullptr, &imguiDescriptorPool_), "Unabled to create imgui descriptor pool");

	const auto imageCount = 2;
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window.window(), true);

	guiInitInfo_.Instance = instance_->instance;
	guiInitInfo_.PhysicalDevice = gpu_->device;
	guiInitInfo_.Device = device_->device;
	guiInitInfo_.QueueFamily = queueFamily_->familyIndex();
	guiInitInfo_.Queue = graphicsQueue_->queue();
	guiInitInfo_.PipelineCache = VK_NULL_HANDLE;
	guiInitInfo_.DescriptorPool = imguiDescriptorPool_;
	guiInitInfo_.Allocator = nullptr;
	guiInitInfo_.MinImageCount = imageCount;
	guiInitInfo_.ImageCount = imageCount;
	guiInitInfo_.CheckVkResultFn = nullptr;
}*/