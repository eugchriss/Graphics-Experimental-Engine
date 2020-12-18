#include "../headers/Framebuffer.h"
#include "../headers/vulkan_utils.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/imgui_impl_glfw.h"
#include <cassert>

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, VkSurfaceKHR surface, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const uint32_t frameCount) :
	gpu_{ gpu },
	device_{ device }
{
	swapchain_ = std::make_unique<vkn::Swapchain>(gpu_, device_, surface, frameCount);
	framebuffers_.resize(std::size(swapchain_->images()));
	createSignals();
	for (const auto& image : swapchain_->images())
	{
		cbs_.emplace_back(cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
	}
	renderArea_.extent = swapchain_->extent();
	for (auto& effect : shaderEffects)
	{
		effect.preload(device);
		effectRefs_.emplace_back(std::ref(effect));
	}

	auto& lastEffectAttachments = shaderEffects.back().outputAttachments();
	const auto& outputColorBufferAttachmentIt = std::find_if(std::begin(lastEffectAttachments), std::end(lastEffectAttachments), [&](const auto& attachment) {return attachment.layoutIndex == 0; });
	assert(outputColorBufferAttachmentIt != std::end(lastEffectAttachments) && "The last shader effect fragment shader is illformed. (Missing output index 0)");

	auto& outputColorBufferAttachment = *outputColorBufferAttachmentIt;
	const_cast<vkn::Shader::Attachment&>(outputColorBufferAttachment).format = swapchain_->imageFormat();
	const_cast<vkn::Shader::Attachment&>(outputColorBufferAttachment).isPresentBuffer = true;
	createRenderpass(shaderEffects);
	auto& renderpassAttachments = renderpass_->attachments();
	auto outputColorBufferAttachmentIndex = std::find_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& attachment) {return attachment.name == outputColorBufferAttachment.name; })->attachmentIndex;
	renderpassAttachments.erase(std::remove_if(std::begin(renderpassAttachments), std::end(renderpassAttachments), [&](const auto& attachment) { return outputColorBufferAttachment.name == attachment.name; }));

	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.height = swapchain_->extent().height;
	fbInfo.width = swapchain_->extent().width;
	fbInfo.renderPass = renderpass_->renderpass();

	auto& swapchainImages = swapchain_->images();
	for (auto i = 0u; i < frameCount; ++i)
	{
		std::vector<VkImageView> views;
		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			auto& image = images_.emplace_back(gpu_, device_, getUsageFlag(attachment), attachment.format, VkExtent3D{ swapchain_->extent().width, swapchain_->extent().height, 1 });
			if (attachment.name == "depth_buffer")
			{
				views.emplace_back(image.getView(VK_IMAGE_ASPECT_DEPTH_BIT));
			}
			else if (attachment.name == "stencil_buffer")
			{
				views.emplace_back(image.getView(VK_IMAGE_ASPECT_STENCIL_BIT));
			}
			else
			{
				views.emplace_back(image.getView(VK_IMAGE_ASPECT_COLOR_BIT));
			}
		}
		views.insert(std::begin(views) + outputColorBufferAttachmentIndex, swapchainImages[i].getView(VK_IMAGE_ASPECT_COLOR_BIT));
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(device_.device, &fbInfo, nullptr, &framebuffers_[i]), "Failed to create the framebuffer");
	}
	for (auto i = 0u; i < std::size(shaderEffects); ++i)
	{
		shaderEffects[i].active(gpu_, device_, renderpass_->renderpass(), i);
	}
}

vkn::Framebuffer::Framebuffer(vkn::Gpu& gpu, vkn::Device& device, vkn::CommandPool& cbPool, std::vector<vkn::ShaderEffect>& shaderEffects, const VkExtent2D& extent, const uint32_t frameCount) :
	gpu_{ gpu },
	device_{ device }
{
	framebuffers_.resize(frameCount);
	createRenderpass(shaderEffects);
	const auto& renderpassAttachments = renderpass_->attachments();
	createSignals();
	for (auto& effect : shaderEffects)
	{
		effect.preload(device);
		effectRefs_.emplace_back(std::ref(effect));
	}
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.flags = 0;
	fbInfo.layers = 1;
	fbInfo.height = swapchain_->extent().height;
	fbInfo.width = swapchain_->extent().width;
	fbInfo.renderPass = renderpass_->renderpass();

	for (auto i = 0u; i < frameCount; ++i)
	{
		std::vector<VkImageView> views;
		views.reserve(std::size(renderpassAttachments));
		for (const auto& attachment : renderpassAttachments)
		{
			images_.emplace_back(gpu_, device_, getUsageFlag(attachment), attachment.format, VkExtent3D{ swapchain_->extent().width, swapchain_->extent().height, 1 });
		}
		fbInfo.attachmentCount = std::size(views);
		fbInfo.pAttachments = std::data(views);

		vkn::error_check(vkCreateFramebuffer(device_.device, &fbInfo, nullptr, &framebuffers_[i]), "Failed to create the framebuffer");
	}
	for (auto i = 0u; i < std::size(shaderEffects); ++i)
	{
		shaderEffects[i].active(gpu_, device_, renderpass_->renderpass(), i);
	}
}

vkn::Framebuffer::Framebuffer(Framebuffer&& other) :gpu_{ other.gpu_ }, device_{ other.device_ }
{
	framebuffers_ = std::move(other.framebuffers_);
	renderpass_ = std::move(other.renderpass_);
	images_ = std::move(other.images_);
	swapchain_ = std::move(other.swapchain_);
	cbs_ = std::move(other.cbs_);
	imageAvailableSignals_ = std::move(other.imageAvailableSignals_);
	renderingFinishedSignals_ = std::move(other.renderingFinishedSignals_);
	renderingOrder_ = move(other.renderingOrder_);
	useGui_ = other.useGui_;
	renderArea_ = other.renderArea_;
	effectRefs_ = std::move(other.effectRefs_);
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

const VkFramebuffer vkn::Framebuffer::frame(const uint32_t index) const
{
	assert(index <= std::size(framebuffers_) && "There is no image at that index");
	return framebuffers_[index];
}

const std::vector<vkn::Pixel> vkn::Framebuffer::frameContent(const uint32_t imageIndex)
{
	return images_[imageIndex].content(gpu_);
}

const float vkn::Framebuffer::rawContentAt(const uint32_t imageIndex, const VkDeviceSize offset)
{
	return images_[imageIndex].rawContentAt(gpu_, offset);
}

const std::vector<float> vkn::Framebuffer::frameRawContent(const uint32_t imageIndex)
{
	return images_[imageIndex].rawContent(gpu_);
}

void vkn::Framebuffer::renderGui(std::function<void()>& gui)
{
	useGui_ = true;
	guiContent_ = gui;
}

void vkn::Framebuffer::setupRendering(vkn::ShaderEffect& effect, const vkn::ShaderCamera& camera, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	renderingOrder_.emplace_back(std::make_tuple(std::ref(effect), std::ref(drawables), std::ref(camera)));
}

void vkn::Framebuffer::setupRendering(vkn::ShaderEffect& effect, const vkn::ShaderCamera& camera, const std::reference_wrapper<gee::Drawable>& drawable)
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
			const auto& [sortedDrawables, textures, materials, drawableMaterials, modelMatrices] = createShaderEffectResource(meshHolder, textureHolder, materialHolder, drawablesRef.get());

			effect.bind(cb);
			bindUniforms(effect, cameraRef.get(),  sampler, textureHolder, materialHolder, textures, materials, drawableMaterials, modelMatrices);
			effect.render(cb, meshHolder, sortedDrawables);

			if (effect.index() < std::size(renderingOrder_) - 1)
			{
				vkCmdNextSubpass(cb.commandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
			}
		}

		if (useGui_)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			guiContent_();
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.commandBuffer());
		}
		renderpass_->end(cb);
		cb.end();

		renderingOrder_.clear();
		useGui_ = false;
	}
}

void vkn::Framebuffer::submitTo(vkn::Queue& graphicsQueue)
{
	if (swapchain_)
	{
		swapchain_->setImageAvailableSignal(imageAvailableSignals_[currentFrame_]);
	}
	graphicsQueue.submit(cbs_[currentFrame_], renderingFinishedSignals_[currentFrame_], imageAvailableSignals_[currentFrame_], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, true);
	if (swapchain_)
	{
		graphicsQueue.present(*swapchain_, renderingFinishedSignals_[currentFrame_]);
	}
	currentFrame_ = (currentFrame_ + 1) % std::size(framebuffers_);
}

void vkn::Framebuffer::setViewport(const float x, const float y, const float width, const float height)
{
	for (auto& effectRef : effectRefs_)
	{
		effectRef.get().setViewport(x, y, width, height);
	}
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

VkImageUsageFlags vkn::Framebuffer::getUsageFlag(const vkn::RenderpassAttachment& attachment)
{
	if (attachment.format == VK_FORMAT_D16_UNORM || attachment.format == VK_FORMAT_D32_SFLOAT ||
		attachment.format == VK_FORMAT_S8_UINT || attachment.format == VK_FORMAT_D16_UNORM_S8_UINT ||
		attachment.format == VK_FORMAT_D24_UNORM_S8_UINT || attachment.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

void vkn::Framebuffer::createRenderpass(const std::vector<vkn::ShaderEffect>& shaderEffects)
{
	vkn::RenderpassBuilder renderpassBuilder{};
	auto& renderpassAttachments = createRenderpassAttachments(renderpassBuilder, shaderEffects);
	createRenderpassSubpasses(renderpassBuilder, shaderEffects, renderpassAttachments);
	createSubpassesDepencies(renderpassBuilder, shaderEffects, renderpassAttachments);

	renderpass_ = std::make_unique<vkn::Renderpass>(renderpassBuilder.get(device_));
}

const std::vector<vkn::RenderpassAttachment> vkn::Framebuffer::createRenderpassAttachments(vkn::RenderpassBuilder& builder, const std::vector<vkn::ShaderEffect>& effects)
{
	std::vector<RenderpassAttachment> renderpassAttachments;
	for (const auto& effect : effects)
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
			attachment.attachmentIndex = builder.addAttachmentT(attachment, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
		else
		{
			attachment.attachmentIndex = builder.addAttachmentT(attachment);
		}
	}

	vkn::RenderpassAttachment depthAttachment{};
	depthAttachment.name = "depth_buffer";
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.attachmentIndex = builder.addAttachmentT(depthAttachment, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	renderpassAttachments.emplace_back(depthAttachment);

	vkn::RenderpassAttachment stencilAttachment{};
	stencilAttachment.name = "stencil_buffer";
	stencilAttachment.format = VK_FORMAT_S8_UINT;

	return renderpassAttachments;
}

void vkn::Framebuffer::createRenderpassSubpasses(vkn::RenderpassBuilder& builder, const std::vector<vkn::ShaderEffect>& effects, const std::vector<RenderpassAttachment>& attachments)
{
	for (const auto& effect : effects)
	{
		vkn::RenderpassBuilder::Subpass::Requirement requirements{};
		const auto& inputAttachments = effect.subpassInputAttachments();
		for (const auto& attachment : inputAttachments)
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == attachment.name; });
			assert(result != std::end(attachments) && "Not all attachments have been registered to the renderpass attachments");
			requirements.addInputAttachment(std::distance(std::begin(attachments), result));
		}

		const auto& outputAttachments = effect.outputAttachments();
		for (const auto& attachment : outputAttachments)
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == attachment.name; });
			assert(result != std::end(attachments) && "Not all attachments have been registered to the renderpass attachments");
			requirements.addColorAttachment(std::distance(std::begin(attachments), result));
		}

		if (effect.hasDepthBuffer())
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == "depth_buffer"; });
			assert(result != std::end(attachments) && "No depth buffer has been registered to the renderpass attachments");
			requirements.addDepthAttachment(std::distance(std::begin(attachments), result));
		}
		if (effect.hasStencilBuffer())
		{
			auto result = std::find_if(std::begin(attachments), std::end(attachments), [&](const auto& rpAttachment) {return rpAttachment.name == "stencil_buffer"; });
			assert(result != std::end(attachments) && "No stencil buffer has been registered to the renderpass attachments");
			requirements.addStencilAttachment(std::distance(std::begin(attachments), result));
		}

		requirements.setPreservedAttachments(attachments);
		builder.addSubpass(requirements);
	}
}

void vkn::Framebuffer::createSubpassesDepencies(vkn::RenderpassBuilder& builder, const std::vector<vkn::ShaderEffect>& effects, const std::vector<RenderpassAttachment>& attachments)
{
}

const vkn::Framebuffer::ShaderEffectResource vkn::Framebuffer::createShaderEffectResource(MeshHolder_t& meshHolder, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	meshHolder.resetOccurences();
	TextureSet_t textureSet;
	MaterialSet_t materialSet;
	std::vector<size_t> drawableMaterials;
	std::vector<glm::mat4> modelMatrices{};
	modelMatrices.reserve(std::size(drawables));
	for (const auto& drawableRef : drawables)
	{
		const auto& drawable = drawableRef.get();
		modelMatrices.push_back(drawable.transform);
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
	return { meshHolder.occurences(), textureSet, materialSet, drawableMaterials, modelMatrices };
}

void vkn::Framebuffer::bindTexture(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const
{
	std::vector<VkImageView> textureViews;
	textureViews.reserve(std::size(textures));
	for (const auto& texture : textures)
	{
		textureViews.push_back(textureHolder.get(texture).getView(VK_IMAGE_ASPECT_COLOR_BIT));
	}
	effect.updateTextures("textures", sampler, textureViews, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Framebuffer::bindSkybox(TextureHolder_t& textureHolder, const TextureSet_t& textures, vkn::ShaderEffect& effect, const VkSampler& sampler) const
{
	const auto& diffuseTex = *textures.begin();
	const auto& view = textureHolder.get(diffuseTex).getView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);
	effect.updateTexture("skybox", sampler, view, VK_SHADER_STAGE_FRAGMENT_BIT);
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
	effect.updateBuffer("Materials", shaderMaterials, VK_SHADER_STAGE_FRAGMENT_BIT);
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

void vkn::Framebuffer::bindUniforms(vkn::ShaderEffect& effect, const vkn::ShaderCamera& camera, VkSampler sampler, TextureHolder_t& textureHolder, MaterialHolder_t& materialHolder, const TextureSet_t& textures, const MaterialSet_t& materials, const std::vector<MaterialKey_t>& drawableMaterials, const std::vector<glm::mat4>& transforms)
{
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
		//bindTexture(textureHolder, textures, effect, sampler);
	}
	if ((requirementFlags & vkn::ShaderEffect::Requirement::Skybox) == vkn::ShaderEffect::Requirement::Skybox)
	{
		bindSkybox(textureHolder, textures, effect, sampler);
	}
}
