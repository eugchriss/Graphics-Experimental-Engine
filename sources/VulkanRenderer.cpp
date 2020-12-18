#include "../headers/VulkanRenderer.h"
#include "../headers/vulkan_utils.h"
#include "../headers/PipelineBuilder.h"
#include "../headers/RenderpassBuilder.h"
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include <algorithm>
#include <string>
#include "../headers/TextureImageFactory.h"

#define ENABLE_VALIDATION_LAYERS
vkn::Renderer::Renderer(gee::Window& window)
{

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
	cbPool_ = std::make_unique<vkn::CommandPool>(*device_, queueFamily_->familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	createSampler();

	buildImguiContext(window);
	meshMemoryLocations_ = std::make_unique<MeshHolder_t>(vkn::MeshMemoryLocationFactory{*gpu_, *device_});
	textureHolder_ = std::make_unique<TextureHolder_t>(vkn::TextureImageFactory{ *gpu_, *device_ });
	materialHolder_ = std::make_unique<MaterialHolder_t>(gee::MaterialHelperFactory<TextureKey_t>{});
}

vkn::Renderer::~Renderer()
{
	device_->idle();
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

	cbPool_ = std::make_unique<vkn::CommandPool>(*device_, queueFamily_->familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

const std::optional<size_t> vkn::Renderer::objectAt(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, const uint32_t x, const uint32_t y)
{
	return std::nullopt;
}

void vkn::Renderer::setWindowMinimized(const bool value)
{
	isWindowMinimized_ = value;
}

void vkn::Renderer::updateGui(std::function<void()> guiContent)
{
	guiContent_ = guiContent;
}

void vkn::Renderer::render(vkn::Framebuffer& fb, vkn::ShaderEffect& effect, std::reference_wrapper<gee::Drawable>& drawable)
{
	fb.setupRendering(effect, shaderCamera_, drawable);
}

void vkn::Renderer::render(vkn::Framebuffer& fb, vkn::ShaderEffect& effect, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	fb.setupRendering(effect, shaderCamera_, drawables);
}

void vkn::Renderer::render(vkn::Framebuffer& fb, std::function<void()>& guiDatas)
{
	fb.renderGui(guiContent_);
}

void vkn::Renderer::draw(vkn::Framebuffer& fb)
{
	fb.render(*meshMemoryLocations_, *textureHolder_, *materialHolder_, sampler_);
	fb.submitTo(*graphicsQueue_);
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
}

void vkn::Renderer::updateCamera(const gee::Camera& camera, const float aspectRatio)
{
	shaderCamera_.position = glm::vec4{camera.position_, 1.0f};
	shaderCamera_.view = camera.pointOfView();
	shaderCamera_.projection = camera.perspectiveProjection(aspectRatio);
	shaderCamera_.projection[1][1] *= -1;
}

std::unique_ptr<vkn::Framebuffer> vkn::Renderer::getFramebuffer(std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount)
{
	return std::make_unique<vkn::Framebuffer>(*gpu_, *device_, surface_, *cbPool_, effects, frameCount);
}

vkn::Framebuffer vkn::Renderer::createFramebuffer(const glm::u32vec2& extent, std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount)
{
	return vkn::Framebuffer{ *gpu_, *device_, *cbPool_, effects, VkExtent2D{extent.x, extent.y}, frameCount};
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

	const auto imageCount = 2;
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
	/*ImGui_ImplVulkan_Init(&init_info, imguiRenderpass_->renderpass());

	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	ImGui_ImplVulkan_CreateFontsTexture(cb.commandBuffer());
	cb.end();
	graphicsQueue_->submit(cb);
	graphicsQueue_->idle();*/
}

void vkn::Renderer::prepareBindingBox(std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	meshMemoryLocations_->get(gee::getCubeMesh().hash() , drawables[0].get().boundingBox().mesh());
	boundingBoxModels_.clear(),
	boundingBoxModels_.reserve(std::size(drawables));
	for (auto i = 0u; i < std::size(drawables); ++i)
	{
		//boundingBoxModels_.push_back(modelMatrices_[i] * drawables[i].get().boundingBox().transformMatrix);
	}
}
