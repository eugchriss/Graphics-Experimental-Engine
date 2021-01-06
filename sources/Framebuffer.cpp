#include "../headers/Framebuffer.h"
#include "../headers/QueueFamily.h"
#include "../headers/vulkan_utils.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/imgui_impl_glfw.h"
#include "../headers/Timer.h"
#include <cassert>
#include <iostream>

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, VkSurfaceKHR surface, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const bool enableGui, ImGui_ImplVulkan_InitInfo guiInfo, const uint32_t frameCount) :
	gpu_{ gpu },
	device_{ device },
	cbPool_{ cbPool },
	guiEnabled_{ enableGui }
{
	swapchain_ = std::make_unique<vkn::Swapchain>(gpu_, device_, surface, frameCount);
	framebuffers_.resize(std::size(swapchain_->images()));
	createSignals();
	for (const auto& image : swapchain_->images())
	{
		cbs_.emplace_back(cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
	}
	renderArea_.extent = swapchain_->extent();
	const auto lastEffectName = shaderEffects.back().name();
	for (auto& effect : shaderEffects)
	{
		effect.preload(device);
		effects_.emplace(effect.name(), std::move(effect));
	}

	auto& lastEffectAttachments = effects_.at(lastEffectName).outputAttachments();
	const auto& outputColorBufferAttachmentIt = std::find_if(std::begin(lastEffectAttachments), std::end(lastEffectAttachments), [&](const auto& attachment) {return attachment.layoutIndex == 0; });
	assert(outputColorBufferAttachmentIt != std::end(lastEffectAttachments) && "The last shader effect fragment shader is illformed. (Missing output index 0)");

	auto& outputColorBufferAttachment = *outputColorBufferAttachmentIt;
	const_cast<vkn::Shader::Attachment&>(outputColorBufferAttachment).format = swapchain_->imageFormat();
	const_cast<vkn::Shader::Attachment&>(outputColorBufferAttachment).isPresentBuffer = true;
	presentAttachmentName_ = outputColorBufferAttachment.name;
	createRenderpass(enableGui);
	if (enableGui)
	{
		auto cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		initGui(cb, guiInfo);
	}
	auto& renderpassAttachments = renderpass_->attachments();
	outputColorBufferAttachmentIndex = std::find_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& attachment) {return attachment.name == outputColorBufferAttachment.name; })->attachmentIndex;
	renderpassAttachments.erase(std::remove_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& attachment) { return outputColorBufferAttachment.name == attachment.name; }));

	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = swapchain_->extent().width;
	fbInfo.height = swapchain_->extent().height;
	fbInfo.renderPass = renderpass_->renderpass();

	createFramebufer(fbInfo, renderpassAttachments, outputColorBufferAttachmentIndex, frameCount);
	auto subpassIndex = 0u;
	for (auto& [name, effect] : effects_)
	{
		effect.setViewport(renderArea_.offset.x, renderArea_.offset.y, renderArea_.extent.width, renderArea_.extent.height);
		effect.active(gpu_, device_, renderpass_->renderpass(), subpassIndex);
		
		auto& effectTweakings = effect.tweakings();
		for (auto& tweaking : effectTweakings)
		{
			shaderTweakings_.emplace_back(std::ref(tweaking));
		}
		++subpassIndex;
	}
}

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const VkExtent2D& extent, const uint32_t frameCount) :
	gpu_{ gpu },
	device_{ device },
	cbPool_{ cbPool },
	renderArea_{ {0,0}, extent }
{
	framebuffers_.resize(frameCount);
	for (auto& effect : shaderEffects)
	{
		effect.preload(device);
		effects_.emplace(effect.name(), std::move(effect));
	}
	createRenderpass(false);
	const auto& renderpassAttachments = renderpass_->attachments();
	createSignals();
	for (auto i = 0u; i < frameCount; ++i)
	{
		cbs_.emplace_back(cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
	}
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = extent.width;
	fbInfo.height = extent.height;
	fbInfo.renderPass = renderpass_->renderpass();

	createFramebufer(fbInfo, renderpassAttachments, frameCount);
	auto subpassIndex = 0u;
	for (auto& [name, effect] : effects_)
	{
		effect.setViewport(renderArea_.offset.x, renderArea_.offset.y, renderArea_.extent.width, renderArea_.extent.height);
		effect.active(gpu_, device_, renderpass_->renderpass(), subpassIndex);
		auto& effectTweakings = effect.tweakings();
		for (auto& tweaking : effectTweakings)
		{
			shaderTweakings_.emplace_back(std::ref(tweaking));
		}
		++subpassIndex;
	}
}


vkn::Framebuffer::~Framebuffer()
{
	for (auto fb : framebuffers_)
	{
		vkDestroyFramebuffer(device_.device, fb, nullptr);
	}
}

#ifndef NDEBUG
void vkn::Framebuffer::setDebugName(const std::string& name)
{
	for (auto& image : images_)
	{
		image.setDebugName(name + "image ");
	}
}
#endif

void vkn::Framebuffer::resize(const glm::u32vec2& size)
{
	renderingOrder_.clear();
	renderArea_.extent.width = size.x;
	renderArea_.extent.height = size.y;

	device_.idle();
	createRenderpass(guiEnabled_);
	auto subpassIndex = 0u;
	for (auto& [name, effect] : effects_)
	{
		effect.setViewport(renderArea_.offset.x, renderArea_.offset.y, renderArea_.extent.width, renderArea_.extent.height);
		effect.active(gpu_, device_, renderpass_->renderpass(), subpassIndex);
		auto& effectTweakings = effect.tweakings();
		for (auto& tweaking : effectTweakings)
		{
			shaderTweakings_.emplace_back(std::ref(tweaking));
		}
		++subpassIndex;
	}

	cbs_.clear();
	auto frameCount = std::size(framebuffers_);
	for (auto i = 0u; i < frameCount; ++i)
	{
		cbs_.emplace_back(cbPool_.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
	}
	imageAvailableSignals_.clear();
	renderingFinishedSignals_.clear();
	for (auto fb : framebuffers_)
	{
		vkDestroyFramebuffer(device_.device, fb, nullptr);
	}
	createSignals();
	framebuffers_.resize(frameCount);

	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.width = size.x;
	fbInfo.height = size.y;
	fbInfo.renderPass = renderpass_->renderpass();
	auto& renderpassAttachments = renderpass_->attachments();

	if (swapchain_)
	{
		vkn::QueueFamily queueFamily{ gpu_, VK_QUEUE_TRANSFER_BIT, 1 };
		auto transferQueue = queueFamily.getQueue(device_);
		auto cb = cbPool_.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		cb.begin();
		swapchain_->resize(cb, VkExtent2D{ size.x, size.y });
		cb.end();
		vkn::Signal swapchainTransitionned{ device_ };
		transferQueue->submit(cb, swapchainTransitionned);
		swapchainTransitionned.waitForSignal();

		renderpassAttachments.erase(std::remove_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& attachment) { return outputColorBufferAttachmentIndex == attachment.attachmentIndex; }));

		createFramebufer(fbInfo, renderpassAttachments, outputColorBufferAttachmentIndex, frameCount);
	}
	else
	{
		createFramebufer(fbInfo, renderpassAttachments, frameCount);
	}

}

const std::vector<vkn::Pixel> vkn::Framebuffer::frameContent(const uint32_t imageIndex)
{
	return images_[imageIndex].content(gpu_);
}

const float vkn::Framebuffer::rawContentAt(const VkDeviceSize offset, const uint32_t imageIndex)
{
	renderingFinishedSignals_[currentFrame_].waitForSignal();
	return images_[imageIndex].rawContentAt(gpu_, offset);
}

const std::vector<float> vkn::Framebuffer::frameRawContent(const uint32_t imageIndex)
{
	return images_[imageIndex].rawContent(gpu_);
}

void vkn::Framebuffer::setupRendering(const std::string& effectName, const vkn::ShaderCamera& camera, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	auto effect = effects_.find(effectName);
	assert(effect != std::end(effects_) && "The effect name doesn t match any registered effect");
	renderingOrder_.emplace_back(std::make_tuple(std::ref(effect->second), std::ref(drawables), std::ref(camera)));
	assert(std::size(renderingOrder_) <= std::size(effects_) && "Number of draw render calls doesn t match the number of effect");
}

void vkn::Framebuffer::setupRendering(const std::string& effectName, const vkn::ShaderCamera& camera, const std::reference_wrapper<gee::Drawable>& drawable)
{
	//renderingOrder_.emplace_back(std::make_tuple(std::ref(effect), std::ref(drawable), std::ref(camera)));
}

void vkn::Framebuffer::render(MeshHolder_t& meshHolder, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const VkSampler& sampler)
{
	if (shouldRender())
	{
		std::sort(std::begin(renderingOrder_), std::end(renderingOrder_), [&](const auto& lhs, const auto& rhs)
			{
				return std::get<0>(lhs).get().index() <= std::get<0>(rhs).get().index();
			});
		auto& cb = cbs_[currentFrame_];
		cb.begin();
		renderpass_->begin(cb, framebuffers_[currentFrame_], renderArea_, VK_SUBPASS_CONTENTS_INLINE);

		for (auto& [effectRef, drawablesRef, cameraRef] : renderingOrder_)
		{
			auto& effect = effectRef.get();
			effect.bind(cb);
			const auto& [sortedDrawables, textures, materials, drawableMaterials, modelMatrices, pointLights] = createShaderEffectResource(meshHolder, textureHolder, materialHolder, drawablesRef.get());
			bindUniforms(effect, cameraRef.get(), sampler, textureHolder, materialHolder, textures, materials, drawableMaterials, modelMatrices, pointLights);
			effect.render(cb, meshHolder, sortedDrawables);

			if (!isLastEffect(effect))
			{
				VkClearValue clear{};
				clear.depthStencil = { 1.0f, 0 };
				renderpass_->clearDepthAttachment(cb, renderArea_, clear);
				vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
			}
		}
		for (auto& [name, effect] : effects_)
		{
			if (effect.isPostProcessEffect())
			{
				effect.bind(cb);
				bindInputTexture(effect, sampler);
				effect.render(cb, meshHolder);
				if (!isLastEffect(effect))
				{
					VkClearValue clear{};
					clear.depthStencil = { 1.0f, 0 };
					renderpass_->clearDepthAttachment(cb, renderArea_, clear);
					vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
				}
			}
		}
		if (guiEnabled_)
		{
			vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.commandBuffer());

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
		renderpass_->end(cb);
		cb.end();

		renderingOrder_.clear();
	}
}

void vkn::Framebuffer::submitTo(vkn::Queue& graphicsQueue)
{
	if (swapchain_)
	{
		swapchain_->setImageAvailableSignal(imageAvailableSignals_[currentFrame_]);
		gee::Timer t{ "rendering time" };
		graphicsQueue.submit(cbs_[currentFrame_], renderingFinishedSignals_[currentFrame_], imageAvailableSignals_[currentFrame_], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, true);
		renderingFinishedSignals_[currentFrame_].waitForSignal();
		std::cout << "rendering time " << t.ellapsedMs() << "ms\n";
		for (auto& image : images_)
		{
			image.setAfterRenderpassLayout();
		}
		graphicsQueue.present(*swapchain_, renderingFinishedSignals_[currentFrame_]);
	}
	else
	{
		graphicsQueue.submit(cbs_[currentFrame_], renderingFinishedSignals_[currentFrame_]);
		for (auto& image : images_)
		{
			image.setAfterRenderpassLayout();
		}
	}
	currentFrame_ = (currentFrame_ + 1) % std::size(framebuffers_);
}

void vkn::Framebuffer::setViewport(const float x, const float y, const float width, const float height)
{
	for (auto& [name, effect] : effects_)
	{
		effect.setViewport(x, y, width, height);
	}
}

vkn::ShaderEffect& vkn::Framebuffer::getEffect(const std::string& name)
{
	auto& effect = effects_.find(name);
	assert(effect != std::end(effects_) && "That effect is not been used by the framebuffer");
	return effect->second;
}

const glm::u32vec2 vkn::Framebuffer::renderArea() const
{
	return glm::u32vec2{ renderArea_.extent.width, renderArea_.extent.height };
}

std::vector<std::reference_wrapper<vkn::Shader::Tweaking>>& vkn::Framebuffer::shaderTweakings()
{
	return shaderTweakings_;
}

void vkn::Framebuffer::initGui(vkn::CommandBuffer& cb, ImGui_ImplVulkan_InitInfo& info)
{
	info.Subpass = std::size(effects_);
	cb.begin();
	ImGui_ImplVulkan_Init(&info, renderpass_->renderpass());
	ImGui_ImplVulkan_CreateFontsTexture(cb.commandBuffer());
	cb.end();

	vkn::QueueFamily queueFamily{ gpu_, VK_QUEUE_TRANSFER_BIT, 1 };
	auto transferQueue = queueFamily.getQueue(device_);
	transferQueue->submit(cb);
	transferQueue->idle();
}

VkImageAspectFlags vkn::Framebuffer::getAspectFlag(const vkn::RenderpassAttachment& attachment)
{
	if (attachment.format == VK_FORMAT_D16_UNORM || attachment.format == VK_FORMAT_D32_SFLOAT)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (attachment.format == VK_FORMAT_S8_UINT)
	{
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if (attachment.format == VK_FORMAT_D16_UNORM_S8_UINT || attachment.format == VK_FORMAT_D24_UNORM_S8_UINT || attachment.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

void vkn::Framebuffer::createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<vkn::RenderpassAttachment>& renderpassAttachments, const uint32_t frameCount)
{
	for (auto i = 0u; i < frameCount; ++i)
	{
		std::vector<VkImageView> views;
		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			attachementImageIndexes_[attachment.name] = std::size(images_);
			images_.emplace_back(gpu_, device_, attachment, VkExtent3D{ renderArea_.extent.width, renderArea_.extent.height, 1 });
			views.emplace_back(images_.back().getView(getAspectFlag(attachment)));
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(device_.device, &fbInfo, nullptr, &framebuffers_[i]), "Failed to create the framebuffer");
	}
}

void vkn::Framebuffer::createFramebufer(VkFramebufferCreateInfo& fbInfo, const std::vector<vkn::RenderpassAttachment>& renderpassAttachments, const uint32_t presentAttachmentIndex, const uint32_t frameCount)
{
	assert(swapchain_ && "This override needs to have the swapchain created first");
	auto& swapchainImages = swapchain_->images();
	for (auto i = 0u; i < frameCount; ++i)
	{
		std::vector<VkImageView> views;
		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			attachementImageIndexes_[attachment.name] = std::size(images_);
			auto& image = images_.emplace_back(gpu_, device_, attachment, VkExtent3D{ swapchain_->extent().width, swapchain_->extent().height, 1 });
			views.emplace_back(image.getView(getAspectFlag(attachment)));
		}
		views.insert(std::begin(views) + presentAttachmentIndex, swapchainImages[i].getView(VK_IMAGE_ASPECT_COLOR_BIT));
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(device_.device, &fbInfo, nullptr, &framebuffers_[i]), "Failed to create the framebuffer");
	}	
}

void vkn::Framebuffer::createSignals()
{
	auto frameCount = std::size(framebuffers_);
	if (swapchain_)
	{
		for (auto i = 0u; i < frameCount; ++i)
		{
			imageAvailableSignals_.emplace_back(device_, true);
			renderingFinishedSignals_.emplace_back(device_, true);
		}
	}
	else
	{
		for (auto i = 0u; i < frameCount; ++i)
		{
			renderingFinishedSignals_.emplace_back(device_, true);
		}
	}
}

bool vkn::Framebuffer::shouldRender()
{
	bool shouldRender{ false };
	if (swapchain_)
	{
		if (imageAvailableSignals_[currentFrame_].signaled() && renderingFinishedSignals_[currentFrame_].signaled())
		{
			renderingFinishedSignals_[currentFrame_].reset();
			shouldRender = true;
		}
	}
	else
	{
		if (renderingFinishedSignals_[currentFrame_].signaled())
		{
			renderingFinishedSignals_[currentFrame_].reset();
			shouldRender = true;
		}
	}
	return shouldRender;
}

void vkn::Framebuffer::createRenderpass(const bool enableGui)
{
	vkn::RenderpassBuilder renderpassBuilder{};
	auto& renderpassAttachments = createRenderpassAttachments(renderpassBuilder);
	auto attachmentDependencies = createRenderpassSubpasses(renderpassBuilder, renderpassAttachments);
	if (enableGui)
	{
		createGuiSubpass(renderpassBuilder, renderpassAttachments, attachmentDependencies);
	}
	createSubpassesDepencies(renderpassBuilder, attachmentDependencies);

	renderpass_ = std::make_unique<vkn::Renderpass>(renderpassBuilder.get(device_));
}

const std::vector<vkn::RenderpassAttachment> vkn::Framebuffer::createRenderpassAttachments(vkn::RenderpassBuilder& builder)
{
	std::vector<RenderpassAttachment> renderpassAttachments;
	for (const auto& [name, effect] : effects_)
	{
		auto& attachments = effect.outputAttachments();
		for (const auto& attachment : attachments)
		{
			auto result = std::find_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& att) { return att.name == attachment.name; });
			if (result == std::end(renderpassAttachments))
			{
				RenderpassAttachment att{};
				att.name = attachment.name;
				att.format = attachment.format;
				att.isUsedForPresent = attachment.isPresentBuffer;
				renderpassAttachments.emplace_back(att);
			}
			else
			{
				result->isUsedForPresent = attachment.isPresentBuffer;
				result->format = attachment.format;
			}
		}
	}

	for (auto& attachment : renderpassAttachments)
	{
		if (attachment.isUsedForPresent)
		{
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			attachment.attachmentIndex = builder.addAttachmentT(attachment);
		}
		else
		{
			attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.attachmentIndex = builder.addAttachmentT(attachment);
		}
	}

	vkn::RenderpassAttachment depthAttachment{};
	depthAttachment.name = "depth_buffer";
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.attachmentIndex = builder.addAttachmentT(depthAttachment);
	renderpassAttachments.emplace_back(depthAttachment);

	vkn::RenderpassAttachment stencilAttachment{};
	stencilAttachment.name = "stencil_buffer";
	stencilAttachment.format = VK_FORMAT_S8_UINT;
	stencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	return renderpassAttachments;
}

const std::unordered_map<std::string, std::vector<vkn::SubpassAttachmentUsage>> vkn::Framebuffer::createRenderpassSubpasses(vkn::RenderpassBuilder& builder, const std::vector<RenderpassAttachment>& attachments)
{
	std::unordered_map<std::string, std::vector<vkn::SubpassAttachmentUsage>> attachmentDependencies;
	auto subpassIndex = 0u;
	for (const auto& [name, effect] : effects_)
	{
		vkn::RenderpassBuilder::Subpass::Requirement requirements{};
		const auto& inputAttachments = effect.subpassInputAttachments();
		for (const auto& attachment : inputAttachments)
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == attachment.name; });
			assert(result != std::end(attachments) && "Not all attachments have been registered to the renderpass attachments");
			requirements.addInputAttachment(std::distance(std::begin(attachments), result));

			vkn::SubpassAttachmentUsage attachmentUsage{};
			attachmentUsage.subpassIndex = subpassIndex;
			attachmentUsage.stageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //Assuming that for now all input attachments are consumed in the frag shader
			attachmentUsage.accessFlag = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			attachmentDependencies[attachment.name].push_back(attachmentUsage);
		}

		const auto& outputAttachments = effect.outputAttachments();
		for (const auto& attachment : outputAttachments)
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == attachment.name; });
			assert(result != std::end(attachments) && "Not all attachments have been registered to the renderpass attachments");
			requirements.addColorAttachment(std::distance(std::begin(attachments), result));

			vkn::SubpassAttachmentUsage attachmentUsage{};
			attachmentUsage.subpassIndex = subpassIndex;
			attachmentUsage.stageFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			attachmentUsage.accessFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			attachmentDependencies[attachment.name].push_back(attachmentUsage);
		}

		if (effect.hasDepthBuffer())
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == "depth_buffer"; });
			assert(result != std::end(attachments) && "No depth buffer has been registered to the renderpass attachments");
			requirements.addDepthAttachment(std::distance(std::begin(attachments), result));

			vkn::SubpassAttachmentUsage attachmentUsage{};
			attachmentUsage.subpassIndex = subpassIndex;
			attachmentUsage.stageFlag = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			attachmentUsage.accessFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			attachmentDependencies["depth_buffer"].push_back(attachmentUsage);
		}
		if (effect.hasStencilBuffer())
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == "stencil_buffer"; });
			assert(result != std::end(attachments) && "No stencil buffer has been registered to the renderpass attachments");
			requirements.addStencilAttachment(std::distance(std::begin(attachments), result));

			vkn::SubpassAttachmentUsage attachmentUsage{};
			attachmentUsage.subpassIndex = subpassIndex;
			attachmentUsage.stageFlag = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			attachmentUsage.accessFlag = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			attachmentDependencies["stencil_buffer"].push_back(attachmentUsage);
		}
		++subpassIndex;
		requirements.setPreservedAttachments(attachments);
		builder.addSubpass(requirements);
	}

	return attachmentDependencies;
}

void vkn::Framebuffer::createSubpassesDepencies(vkn::RenderpassBuilder& builder, const std::unordered_map<std::string, std::vector<vkn::SubpassAttachmentUsage>>& attachmentDependencies)
{
	for (const auto& [attachmentName, attachmentUsages] : attachmentDependencies)
	{
		if (std::size(attachmentUsages) > 1)
		{
			for (auto i = 0u; i < std::size(attachmentUsages) - 1; ++i)
			{
				builder.addDependecy(attachmentUsages[i], attachmentUsages[i + 1]);
			}
		}
	}
}

const vkn::Framebuffer::ShaderEffectResource vkn::Framebuffer::createShaderEffectResource(MeshHolder_t& meshHolder, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	meshHolder.resetOccurences();
	TextureSet_t textureSet;
	MaterialSet_t materialSet;
	std::vector<size_t> drawableMaterials;
	std::vector<glm::mat4> modelMatrices{};
	std::vector<gee::ShaderPointLight> pointLights{};

	modelMatrices.reserve(std::size(drawables));
	pointLights.reserve(std::size(drawables));
	for (const auto& drawableRef : drawables)
	{
		auto& drawable = drawableRef.get();
		modelMatrices.push_back(drawable.getTransform());
		if (drawable.hasLightComponent())
		{
			const auto& light = drawable.light();
			gee::ShaderPointLight shaderLight{};
			shaderLight.position = glm::vec4{ light.position, 0.0f };
			shaderLight.ambient = glm::vec4{ light.ambient, 1.0f };
			shaderLight.diffuse = glm::vec4{ light.diffuse, 1.0f };
			shaderLight.specular = glm::vec4{ light.specular, 1.0f };
			shaderLight.linear = light.linear;
			shaderLight.quadratic = light.quadratic;

			pointLights.push_back(shaderLight);
		}
		meshHolder.get(drawable.mesh.hash(), drawable.mesh);

		const auto& materialHash = drawable.mesh.material().hash;
		const auto& material = materialHolder.get(materialHash, textureHolder, drawable.mesh.material());
		drawableMaterials.emplace_back(materialHash);
		textureSet.emplace(material.diffuse);
		textureSet.emplace(material.normal);
		textureSet.emplace(material.specular);

		if (std::find(std::begin(materialSet), std::end(materialSet), materialHash) == std::end(materialSet))
		{
			materialSet.emplace_back(materialHash);
		}
	}
	return { meshHolder.occurences(), textureSet, materialSet, drawableMaterials, modelMatrices, pointLights };
}

void vkn::Framebuffer::bindTexture(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const
{
	std::vector<VkImageView> textureViews;
	textureViews.reserve(std::size(textures));
	for (const auto& texture : textures)
	{
		textureViews.push_back(textureHolder.get(texture).getView(VK_IMAGE_ASPECT_COLOR_BIT));
	}
	auto textureName = vkn::ShaderEffect::requirement_map[vkn::ShaderEffect::Requirement::Texture];
	effect.updateTextures(textureName, sampler, textureViews, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindSkybox(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const
{
	const auto& diffuseTex = *textures.begin();
	const auto& view = textureHolder.get(diffuseTex).getView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);
	auto skyboxName = vkn::ShaderEffect::requirement_map[vkn::ShaderEffect::Requirement::Skybox];
	effect.updateTexture(skyboxName, sampler, view, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindMaterial(MaterialHolder_t& materialHolder, const TextureSet_t textures, const MaterialSet_t& materials, vkn::ShaderEffect& effect) const
{
	std::vector<gee::ShaderMaterial> shaderMaterials;
	shaderMaterials.reserve(std::size(materials));
	for (const auto& material : materials)
	{
		gee::ShaderMaterial shaderMaterial{};
		shaderMaterial.diffuseIndex = indexOf(textures, materialHolder.get(material).diffuse);
		shaderMaterial.normalIndex = indexOf(textures, materialHolder.get(material).normal);
		shaderMaterial.specularIndex = indexOf(textures, materialHolder.get(material).specular);
		shaderMaterials.push_back(shaderMaterial);
	}
	auto materialName = vkn::ShaderEffect::requirement_map[vkn::ShaderEffect::Requirement::Material];
	effect.updateBuffer(materialName, shaderMaterials, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindInputTexture(vkn::ShaderEffect& effect, const VkSampler& sampler)
{
	const auto& texturesNames = effect.inputTexturesNames();
	for (const auto& name : texturesNames)
	{
		if (name.substr(std::size(vkn::ShaderEffect::attachmentPrefix)) == presentAttachmentName_)
		{
			auto view = swapchain_->images()[currentFrame_].getView(VK_IMAGE_ASPECT_COLOR_BIT);
			effect.updateTexture(name, sampler, view, VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		else
		{
			auto textureIndex = attachementImageIndexes_.find(name.substr(std::size(vkn::ShaderEffect::attachmentPrefix)));
			assert(textureIndex != std::end(attachementImageIndexes_) && "The texture doesn t match any renderpass attachment");
			auto view = images_[textureIndex->second].getView(VK_IMAGE_ASPECT_COLOR_BIT);
			effect.updateTexture(name, sampler, view, VK_SHADER_STAGE_FRAGMENT_BIT);
		}
	}
}

void vkn::Framebuffer::bindLight(vkn::ShaderEffect& effect, const std::vector<gee::ShaderPointLight>& pointLights)
{
	auto pointLightName = vkn::ShaderEffect::requirement_map[vkn::ShaderEffect::Requirement::Light];
	effect.updateBuffer(pointLightName, pointLights, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindDrawableMaterial(MaterialHolder_t& materialHolder, const std::vector<Hash_t>& materials, vkn::ShaderEffect& effect) const
{
	std::vector<uint32_t> drawableMaterials;
	drawableMaterials.reserve(std::size(materials));
	for (const auto& materialHash : materials)
	{
		drawableMaterials.push_back(materialHolder.indexOf(materialHash));
	}
	effect.updateBuffer("DrawableMaterial", drawableMaterials, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindUniforms(vkn::ShaderEffect& effect, const vkn::ShaderCamera& camera, VkSampler sampler, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const TextureSet_t& textures, const MaterialSet_t& materials, const std::vector<MaterialKey_t>& drawableMaterials, const std::vector<glm::mat4>& transforms, const std::vector<gee::ShaderPointLight>& pointLights)
{
	bindInputTexture(effect, sampler);
	auto requirementFlags = effect.getRequirement();
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Texture) == vkn::ShaderEffect::Requirement::Texture)
	{
		bindTexture(textureHolder, textures, effect, sampler);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Transform) == vkn::ShaderEffect::Requirement::Transform)
	{
		effect.updateBuffer("Model_Matrix", transforms, VK_SHADER_STAGE_VERTEX_BIT);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Camera) == vkn::ShaderEffect::Requirement::Camera)
	{
		effect.updateBuffer("Camera", camera, VK_SHADER_STAGE_VERTEX_BIT);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Material) == vkn::ShaderEffect::Requirement::Material)
	{
		bindMaterial(materialHolder, textures, materials, effect);
		bindDrawableMaterial(materialHolder, drawableMaterials, effect);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Light) == vkn::ShaderEffect::Requirement::Light)
	{
		bindLight(effect, pointLights);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Skybox) == vkn::ShaderEffect::Requirement::Skybox)
	{
		bindSkybox(textureHolder, textures, effect, sampler);
	}
}

void vkn::Framebuffer::createGuiSubpass(vkn::RenderpassBuilder& builder, const std::vector<RenderpassAttachment>& attachments, std::unordered_map<std::string, std::vector<vkn::SubpassAttachmentUsage>>& attachmentDependencies)
{
	auto presentAttachment = std::find_if(std::begin(attachments), std::end(attachments), [](const auto& attachment) { return attachment.isUsedForPresent; });
	assert(presentAttachment != std::end(attachments) && "The renderpass has no present attachment created");
	vkn::RenderpassBuilder::Subpass::Requirement requirement;
	requirement.addColorAttachment(std::distance(std::begin(attachments), presentAttachment));
	requirement.setPreservedAttachments(attachments);

	vkn::SubpassAttachmentUsage attachmentUsage{};
	attachmentUsage.subpassIndex = std::size(effects_);
	attachmentUsage.stageFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	attachmentUsage.accessFlag = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	attachmentDependencies[presentAttachment->name].push_back(attachmentUsage);
	builder.addSubpass(requirement);
}

const bool vkn::Framebuffer::isLastEffect(const vkn::ShaderEffect& effect) const
{
	return effect.index() ==  (std::size(effects_) - 1);
}