#include "../headers/VulkanRenderer.h"
#include "../headers/vulkan_utils.h"
#include "../headers/PipelineBuilder.h"
#include "../headers/RenderpassBuilder.h"
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include <algorithm>
#include <string>


#define ENABLE_VALIDATION_LAYERS
vkn::Renderer::Renderer(gee::Window& window) :renderArea_{ 0, 0, window.size().x, window.size().y }
{
	viewport_ = VkViewport{ 0.0f, 0.0f, static_cast<float>(window.size().x), static_cast<float>(window.size().y), 0.0f, 1.0f };
	isWindowMinimized_ = viewport_.width == 0 && viewport_.height == 0;
#ifdef ENABLE_VALIDATION_LAYERS
	instance_ = std::make_unique<vkn::Instance>(std::initializer_list<const char*>{ "VK_LAYER_KHRONOS_validation" });
#else
	instance_ = std::make_unique<vkn::Instance>(std::initializer_list<const char*>{});
#endif // ENABLE_VALIDATION_LAYERS

	VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessenger_ = std::make_unique<vkn::DebugMessenger>(*instance_, severityFlags, messageTypeFlags);

	auto gpus = vkn::Gpu::getAvailbleGpus(*instance_);
	if (std::size(gpus) < 1)
	{
		throw std::runtime_error{ "There is no avaible gpu on this system" };
	}
	gpu_ = std::make_unique<vkn::Gpu>(gpus[0]);
	checkGpuCompability(*gpu_);

	vkn::error_check(glfwCreateWindowSurface(instance_->instance, window.window(), nullptr, &surface_), "unable to create a presentable surface for the window");
	queueFamily_ = std::make_unique<vkn::QueueFamily>(*gpu_, VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_TRANSFER_BIT, surface_, 2);
	device_ = std::make_unique<vkn::Device>(*gpu_, std::initializer_list<const char*>{"VK_KHR_swapchain", "VK_EXT_descriptor_indexing", "VK_KHR_maintenance3"}, * queueFamily_);
	graphicsQueue_ = queueFamily_->getQueue(*device_);
	transferQueue_ = queueFamily_->getQueue(*device_);
	swapchain_ = std::make_unique<vkn::Swapchain>(*gpu_, *device_, surface_);
	buildShaderTechnique();
	cbPool_ = std::make_unique<vkn::CommandPool>(*device_, queueFamily_->familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	createSampler();

	imageCount_ = std::size(swapchain_->images());
	for (auto i = 0; i < imageCount_; ++i)
	{
		cbs_.emplace_back(cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		imageAvailableSignals_.emplace_back(*device_, true);
		renderingFinishedSignals_.emplace_back(*device_, true);
	}
	buildImguiContext(window);
}

vkn::Renderer::~Renderer()
{
	device_->idle();
	imageAvailableSignals_.clear();
	renderingFinishedSignals_.clear();
	swapchain_.reset();
	vkDestroySurfaceKHR(instance_->instance, surface_, nullptr);
	vkDestroySampler(device_->device, sampler_, nullptr);
	vkDestroyDescriptorPool(device_->device, imguiDescriptorPool_, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::ostream& vkn::Renderer::getGpuInfo(std::ostream& os) const
{
	os << *gpu_;
	return os;
}

void vkn::Renderer::resize()
{
	isWindowMinimized_ = false;
	device_->idle();
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vkn::error_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu_->device, surface_, &surfaceCapabilities), "Failed to get surface capabilities");
	auto newExtent = surfaceCapabilities.currentExtent;
	renderArea_ = VkRect2D{ 0, 0, newExtent };
	viewport_ = VkViewport{ 0.0f, 0.0f, static_cast<float>(newExtent.width), static_cast<float>(newExtent.height), 0.0f, 1.0f };

	cbPool_ = std::make_unique<vkn::CommandPool>(*device_, queueFamily_->familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	cbs_.clear();
	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	swapchain_->resize(cb, newExtent);
	cb.end();
	vkn::Signal layoutTransitionned{ *device_ };
	transferQueue_->submit(cb, layoutTransitionned);

	buildShaderTechnique();
	setBackgroundColor(backgroundColor_);
	skyboxTechnique_->updatePipelineTexture("skybox", sampler_, skybox_->getView(), VK_SHADER_STAGE_FRAGMENT_BIT);
	imageAvailableSignals_.clear();
	renderingFinishedSignals_.clear();
	imageCount_ = std::size(swapchain_->images());
	for (auto& image : swapchain_->images())
	{
		cbs_.emplace_back(cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		imageAvailableSignals_.emplace_back(*device_, true);
		renderingFinishedSignals_.emplace_back(*device_, true);
	}
	layoutTransitionned.waitForSignal();
}

void vkn::Renderer::enableSkybox(bool value)
{
	skyboxEnbaled_ = value;
}

void vkn::Renderer::setSkybox(const std::array<std::string, 6>& skyboxPaths)
{
	skybox_ = std::make_unique<vkn::Skybox>(*gpu_, *device_, skyboxPaths);
	skyboxTechnique_->updatePipelineTexture("skybox", sampler_, skybox_->getView(), VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Renderer::setWindowMinimized(const bool value)
{
	isWindowMinimized_ = value;
}

void vkn::Renderer::setRenderArea(const VkRect2D renderArea)
{
	renderArea_ = renderArea;
}

bool vkn::Renderer::addMesh(const gee::Mesh& mesh)
{
	auto result = meshesMemory_.find(mesh.hash());
	if (result == std::end(meshesMemory_))
	{
		meshesMemory_.emplace(mesh.hash(), addMeshToMemory(mesh));
		return true;
	}
	return false;
}

const size_t vkn::Renderer::addMaterial(const gee::Material& material)
{
	auto result = shaderMaterials_.find(material.hash);
	if (result == std::end(shaderMaterials_))
	{
		gee::ShaderMaterial shaderMaterial{};

		shaderMaterial.diffuseIndex = addTexture(material.diffuseTex);
		shaderMaterial.normalIndex = addTexture(material.normalTex);
		shaderMaterial.specularIndex = addTexture(material.specularTex);

		shaderMaterials_.emplace(material.hash, shaderMaterial);
		return std::size(shaderMaterials_) - 1;
	}
	return std::distance(std::begin(shaderMaterials_), result);
}

const size_t vkn::Renderer::addTexture(const gee::Texture& texture)
{
	auto result = std::find_if(std::begin(textures_), std::end(textures_), [&](const auto& pair) { return pair.first == texture.hash(); });
	if (result == std::end(textures_))
	{
		auto& image = createImageFromTexture(texture);
		textures_.emplace_back(texture.hash(), std::move(image));
		return std::size(textures_) - 1;
	}
	return std::distance(std::begin(textures_), result);
}

void vkn::Renderer::updateGui(std::function<void()> guiContent)
{
	guiContent_ = guiContent;
}

void vkn::Renderer::draw(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, std::vector<std::reference_wrapper<gee::Drawable>>& lights)
{
	if (!isWindowMinimized_)
	{
		if (imageAvailableSignals_[currentFrame_].signaled() && renderingFinishedSignals_[currentFrame_].signaled())
		{
			renderingFinishedSignals_[currentFrame_].reset();
			const auto& sortedDrawables = createSortedDrawables(drawables);
			if(std::size(lights) > 0)
			{
				bindLights(lights);
			}
			if (std::size(drawables) > 0)
			{
				bindTexture(textures_);
				bindShaderMaterial(shaderMaterials_);
				forwardRendering_->updatePipelineBuffer("Model_Matrix", modelMatrices_, VK_SHADER_STAGE_VERTEX_BIT);
				forwardRendering_->updatePipelineBuffer("Colors", drawablesColors_, VK_SHADER_STAGE_VERTEX_BIT);
				if (showBindingBox_)
				{
					prepareBindingBox(drawables);
				}
			}
			record(sortedDrawables);
			submit();
		}
	}
}

void vkn::Renderer::bindTexture(std::vector<std::pair<size_t, vkn::Image>>& textures)
{
	std::vector<VkImageView> textureViews;
	textureViews.reserve(std::size(textures));

	for (auto& [hashKey, texture] : textures)
	{
		textureViews.push_back(texture.getView(VK_IMAGE_ASPECT_COLOR_BIT));
	}
	forwardRendering_->updatePipelineTextures("textures", sampler_, textureViews, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Renderer::bindShaderMaterial(const std::unordered_map<size_t, gee::ShaderMaterial>& materials)
{
	std::vector<gee::ShaderMaterial> shaderMaterials;
	shaderMaterials.reserve(std::size(materials));
	for (const auto& [hashID, material] : materials)
	{
		shaderMaterials.push_back(material);
	}
	forwardRendering_->updatePipelineBuffer("Materials", shaderMaterials, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Renderer::bindLights(std::vector<std::reference_wrapper<gee::Drawable>>& lights)
{
	std::vector<gee::ShaderPointLight> shaderLights;
	shaderLights.reserve(std::size(lights));
	for (const auto& lightRef : lights)
	{
		const auto& light = lightRef.get().light();
		gee::ShaderPointLight l;
		l.position = glm::vec4{ light.position, 1.0f };
		l.ambient = glm::vec4{ light.ambient, 1.0f };
		l.diffuse = glm::vec4{ light.diffuse, 1.0f };
		l.specular = glm::vec4{ light.specular, 1.0f };
		l.linear = light.linear;
		l.quadratic = light.quadratic;
		shaderLights.push_back(l);

	}
	forwardRendering_->updatePipelineBuffer("PointLights", shaderLights, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void vkn::Renderer::updateCamera(const gee::Camera& camera, const float aspectRatio)
{
	shaderCamera_.position = glm::vec4{camera.position_, 1.0f};
	shaderCamera_.view = camera.pointOfView();
	shaderCamera_.projection = camera.perspectiveProjection(aspectRatio);
	shaderCamera_.projection[1][1] *= -1;
	forwardRendering_->updatePipelineBuffer("Camera", shaderCamera_, VK_SHADER_STAGE_VERTEX_BIT);
	if (showBindingBox_)
	{
		boundingBoxRendering_->updatePipelineBuffer("Camera", shaderCamera_, VK_SHADER_STAGE_VERTEX_BIT);
	}
	if (skyboxEnbaled_)
	{
		ShaderCamera info;
		info.position = shaderCamera_.position;
		info.projection = shaderCamera_.projection;
		info.view = glm::mat4{ glm::mat3{camera.pointOfView()} };
		skyboxTechnique_->updatePipelineBuffer("Camera", info, VK_SHADER_STAGE_VERTEX_BIT);
	}
}

void vkn::Renderer::setBackgroundColor(const glm::vec3& color)
{
	backgroundColor_ = color;
	forwardRendering_->setClearColor(backgroundColor_);
}

void vkn::Renderer::checkGpuCompability(const vkn::Gpu& gpu)
{
	auto& features = gpu.enabledFeatures();
	auto descriptorIndexingFeatures = reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures*>(features.pNext);
	if (!descriptorIndexingFeatures->descriptorBindingUniformBufferUpdateAfterBind || !descriptorIndexingFeatures->descriptorBindingSampledImageUpdateAfterBind
		|| !descriptorIndexingFeatures->runtimeDescriptorArray || !descriptorIndexingFeatures->descriptorBindingPartiallyBound)
	{
		throw std::runtime_error{ "There is no compatible gpu available." };
	}
}

void vkn::Renderer::buildShaderTechnique()
{
	VkImageLayout initialLayout{VK_IMAGE_LAYOUT_UNDEFINED};
	VkAttachmentLoadOp loadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
	if (skyboxEnbaled_)
	{
		auto renderpassBuilder = vkn::RenderpassBuilder::getDefaultColorDepthResolveRenderpass(*device_, swapchain_->imageFormat(), loadOp, initialLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		auto pipelineBuilder = vkn::PipelineBuilder::getDefault3DPipeline(*gpu_, *device_, "../assets/Shaders/skybox/vert.spv", "../assets/Shaders/skybox/frag.spv");
		skyboxTechnique_ = std::make_unique<vkn::ShaderTechnique>(renderpassBuilder, pipelineBuilder, *swapchain_);
		initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	auto renderpassBuilder = vkn::RenderpassBuilder::getDefaultColorDepthResolveRenderpass(*device_, swapchain_->imageFormat(), loadOp, initialLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	auto pipelineBuilder = vkn::PipelineBuilder::getDefault3DPipeline(*gpu_, *device_, "../assets/Shaders/vert.spv", "../assets/Shaders/frag.spv");
	forwardRendering_ = std::make_unique<vkn::ShaderTechnique>(renderpassBuilder, pipelineBuilder, *swapchain_);
	
	if (showBindingBox_)
	{
		auto boundingBoxRenderpassBuilder = vkn::RenderpassBuilder::getDefaultColorDepthResolveRenderpass(*device_, swapchain_->imageFormat(), loadOp, initialLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		auto boundingBoxPipelineBuilder = vkn::PipelineBuilder::getDefault3DPipeline(*gpu_, *device_, "../assets/Shaders/boundingBox/vert.spv", "../assets/Shaders/boundingBox/frag.spv");
		boundingBoxPipelineBuilder.addRaterizationStage(VK_POLYGON_MODE_LINE);
		boundingBoxPipelineBuilder.lineWidth = 5.0f;
		boundingBoxRendering_ = std::make_unique<vkn::ShaderTechnique>(boundingBoxRenderpassBuilder, boundingBoxPipelineBuilder, *swapchain_);
	}
	auto imguiRenderpassBuilder = vkn::RenderpassBuilder::getDefaultColorDepthResolveRenderpass(*device_, swapchain_->imageFormat(), loadOp, initialLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	imguiRenderpass_ = std::make_unique<vkn::Renderpass>(imguiRenderpassBuilder.get());
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
	vkn::error_check(vkCreateSampler(device_->device, &samplerInfo, nullptr, &sampler_), "Failed to create the sampler");
}

void vkn::Renderer::buildImguiContext(const gee::Window& window)
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

	const auto imageCount = std::size(swapchain_->images());
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(window.window(), true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance_->instance;
	init_info.PhysicalDevice = gpu_->device;
	init_info.Device = device_->device;
	init_info.QueueFamily = queueFamily_->familyIndex();
	init_info.Queue = graphicsQueue_->queue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = imguiDescriptorPool_;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = imageCount;
	init_info.ImageCount = imageCount;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, imguiRenderpass_->renderpass());

	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	ImGui_ImplVulkan_CreateFontsTexture(cb.commandBuffer());
	cb.end();
	graphicsQueue_->submit(cb);
	graphicsQueue_->idle();
}

void vkn::Renderer::record(const std::unordered_map<size_t, uint64_t>& sortedDrawables)
{
	auto& cb = cbs_[currentFrame_];
	cb.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	if (skyboxEnbaled_)
	{
		skyboxTechnique_->execute(cb, currentFrame_, viewport_, renderArea_, [&]() 
			{
				VkDeviceSize offset{ 0 };
				vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &skybox_->vertexBuffer(), &offset);
				vkCmdBindIndexBuffer(cb.commandBuffer(), skybox_->indexBuffer(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cb.commandBuffer(), skybox_->indexCount(), 1, 0, 0, 0);
			});
	}

	forwardRendering_->execute(cb, currentFrame_, viewport_, renderArea_, [&]()
		{
			uint32_t instanceIndex{ 0 };
			for (const auto [meshHashKey, instanceCount] : sortedDrawables)
			{
				VkDeviceSize offset{ 0 };
				auto& memoryLocation = meshesMemory_.at(meshHashKey);

				forwardRendering_->pipelinePushConstant(cb, "modelIndex", meshesShaderIndices_[instanceIndex].modelIndices);
				forwardRendering_->pipelinePushConstant(cb, "materialIndex", meshesShaderIndices_[instanceIndex].materialIndex);
				vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &memoryLocation.vertexBuffer.buffer, &offset);
				vkCmdBindIndexBuffer(cb.commandBuffer(), memoryLocation.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cb.commandBuffer(), memoryLocation.indicesCount, instanceCount, 0, 0, 0);
				instanceIndex += instanceCount;
			}
		});
	if (showBindingBox_)
	{
		boundingBoxRendering_->execute(cb, currentFrame_, viewport_, renderArea_, [&]()
			{
				VkDeviceSize offset{ 0 };
				vkCmdBindVertexBuffers(cb.commandBuffer(), 0, 1, &boundingBoxMemoryLocation_->vertexBuffer.buffer, &offset);
				vkCmdBindIndexBuffer(cb.commandBuffer(), boundingBoxMemoryLocation_->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(cb.commandBuffer(), boundingBoxMemoryLocation_->indicesCount, std::size(boundingBoxModels_), 0, 0, 0);
			});
	}
	imguiRenderpass_->begin(cb, forwardRendering_->framebuffer(currentFrame_), renderArea_, VK_SUBPASS_CONTENTS_INLINE);
	guiContent_();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.commandBuffer());
	imguiRenderpass_->end(cb);

	cb.end();
}

void vkn::Renderer::submit()
{
	swapchain_->setImageAvailableSignal(imageAvailableSignals_[currentFrame_]);
	graphicsQueue_->submit(cbs_[currentFrame_], renderingFinishedSignals_[currentFrame_], imageAvailableSignals_[currentFrame_], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, true);
	graphicsQueue_->present(*swapchain_, renderingFinishedSignals_[currentFrame_]);
	currentFrame_ = (currentFrame_ + 1) % imageCount_;
}

vkn::Image vkn::Renderer::createImageFromTexture(const gee::Texture& texture)
{
	const auto& datas = texture.pixels();
	const auto textureSize = std::size(datas);
	vkn::Buffer temp{ *device_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, textureSize };
	vkn::DeviceMemory memory{ *gpu_, *device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, temp.getMemorySize() };
	temp.bind(memory);
	temp.add(datas);

	VkFormat imageFormat{ VK_FORMAT_R8G8B8A8_SRGB };
	if (texture.colorSpace() == gee::Texture::ColorSpace::LINEAR)
	{
		imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	}

	vkn::Image image{ *gpu_, *device_, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageFormat, VkExtent3D{texture.width(), texture.height(), 1} };

	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	cb.begin();
	image.copyFromBuffer(cb, VK_IMAGE_ASPECT_COLOR_BIT, temp, 0);
	image.transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cb.end();

	vkn::Signal imageReady{ *device_ };
	transferQueue_->submit(cb, imageReady);
	imageReady.waitForSignal();
	auto test = image.rawContent(*gpu_);
	return std::move(image);
}

const std::unordered_map<size_t, uint64_t> vkn::Renderer::createSortedDrawables(std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	modelMatrices_.clear();
	drawablesColors_.clear();
	meshesShaderIndices_.clear();
	modelMatrices_.reserve(std::size(drawables));
	std::unordered_map<size_t, uint64_t> sortedDrawables;
	std::sort(std::begin(drawables), std::end(drawables), [&](const auto& lhs, const auto& rhs)
		{
			return lhs.get().mesh.hash() < rhs.get().mesh.hash();
		});

	for (const auto& drawableRef : drawables)
	{
		const auto& drawable = drawableRef.get();
		modelMatrices_.emplace_back(getModelMatrix(drawable));
		drawablesColors_.emplace_back(drawable.color);
		if (addMesh(drawable.mesh))
		{
			sortedDrawables[drawable.mesh.hash()] = 1;
		}
		else
		{
			++sortedDrawables[drawable.mesh.hash()];
		}
		const auto materialIndice = addMaterial(drawable.mesh.material());

		MeshIndices shaderIndice;
		shaderIndice.modelIndices = std::size(modelMatrices_) - 1;
		shaderIndice.materialIndex = materialIndice;
		meshesShaderIndices_.emplace_back(shaderIndice);
	}
	return std::move(sortedDrawables);
}

const glm::mat4 vkn::Renderer::getModelMatrix(const gee::Drawable& drawable) const
{
	glm::mat4 mat{ 1.0f };
	mat = glm::translate(mat, drawable.position);
	mat = glm::scale(mat, drawable.size);
	mat = glm::rotate(mat, drawable.rotation.x, glm::vec3{ 1.0, 0.0f, 0.0f });
	mat = glm::rotate(mat, drawable.rotation.y, glm::vec3{ 0.0, 1.0f, 0.0f });
	mat = glm::rotate(mat, drawable.rotation.z, glm::vec3{ 0.0, 0.0f, 1.0f });
	return mat;
}

void vkn::Renderer::prepareBindingBox(std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	if (!boundingBoxMemoryLocation_)
	{
		boundingBoxMemoryLocation_ = std::make_unique<vkn::MemoryLocation>(addMeshToMemory(drawables[0].get().boundingBox().mesh()));
	}
	boundingBoxModels_.clear(),
	boundingBoxModels_.reserve(std::size(modelMatrices_));
	for (auto i = 0u; i < std::size(drawables); ++i)
	{
		boundingBoxModels_.push_back(modelMatrices_[i] * drawables[i].get().boundingBox().transformMatrix);
	}
	boundingBoxRendering_->updatePipelineBuffer("Model_Matrix", boundingBoxModels_, VK_SHADER_STAGE_VERTEX_BIT);
}

vkn::MemoryLocation vkn::Renderer::addMeshToMemory(const gee::AbstractMesh& mesh)
{
	const auto& vertices = mesh.vertices();
	auto verticesSize = std::size(vertices) * sizeof(gee::Vertex);
	vkn::Buffer vertexBuffer{ *device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesSize };

	const auto& indices = mesh.indices();
	auto indicesSize = std::size(indices) * sizeof(uint32_t);
	vkn::Buffer indexBuffer{ *device_, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indicesSize };

	vkn::DeviceMemory memory{ *gpu_, *device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.getMemorySize() + indexBuffer.getMemorySize() };
	vertexBuffer.bind(memory);
	indexBuffer.bind(memory);

	vertexBuffer.add(vertices);
	indexBuffer.add(indices);
	return vkn::MemoryLocation{ std::move(memory), std::move(vertexBuffer), std::move(indexBuffer), static_cast<uint32_t>(std::size(indices)) };
}