#include <string>
#include "../headers/VulkanRenderer.h"
#include "../headers/imgui_impl_glfw.h"
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

	std::vector<vkn::ShaderEffect> pixelPerfectEffect;
	pixelPerfectEffect.emplace_back("pixel perfect", "../assets/Shaders/pixelPerfect/vert.spv", "../assets/Shaders/pixelPerfect/frag.spv");
	pixelPerfectFramebuffer_ = std::make_unique<vkn::Framebuffer>(*gpu_, *device_, *cbPool_, pixelPerfectEffect, VkExtent2D{ window.size().x, window.size().y }, 1);
}

vkn::Renderer::~Renderer()
{
	device_->idle();
	pixelPerfectFramebuffer_.reset();
	mainFramebuffer_.reset();
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

const std::optional<size_t> vkn::Renderer::objectAt(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, const uint32_t x, const uint32_t y)
{
	pixelPerfectFramebuffer_->setupRendering("pixel perfect", shaderCamera_, drawables);
	pixelPerfectFramebuffer_->render(*meshMemoryLocations_, *textureHolder_, *materialHolder_, sampler_);
	pixelPerfectFramebuffer_->submitTo(*graphicsQueue_);
	auto pixel = pixelPerfectFramebuffer_->rawContentAt((x + y * pixelPerfectFramebuffer_->renderArea().x ) * 4);
	if (pixel == 0.0f)
	{
		return std::nullopt;
	}
	else
	{
		return std::make_optional(pixel * 255 - 1);
	}
}

void vkn::Renderer::render(const std::string& effectName, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	if (!isWindowMinimized_)
	{
		assert(mainFramebuffer_ && "the main framebuffer has not been created yet");
		mainFramebuffer_->setupRendering(effectName, shaderCamera_, drawables);
	}
}

void vkn::Renderer::render(vkn::Framebuffer& fb, const std::string& effectName, std::reference_wrapper<gee::Drawable>& drawable)
{
	fb.setupRendering(effectName, shaderCamera_, drawable);
}

void vkn::Renderer::render(vkn::Framebuffer& fb, const std::string& effectName, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables)
{
	fb.setupRendering(effectName, shaderCamera_, drawables);
}

float vkn::Renderer::draw(vkn::Framebuffer& fb)
{
	if (!isWindowMinimized_)
	{
		fb.render(*meshMemoryLocations_, *textureHolder_, *materialHolder_, sampler_);
		return fb.submitTo(*graphicsQueue_);
	}
	return 0.0f;
}

void vkn::Renderer::setViewport(const float x, const float y, const float width, const float height)
{
	assert(mainFramebuffer_ && "The main framebuffer has not been created yet");
	mainFramebuffer_->setViewport(x, y, width, height);
}

float vkn::Renderer::draw()
{
	if (!isWindowMinimized_)
	{
		assert(mainFramebuffer_ && "the main framebuffer has not been created yet");
		mainFramebuffer_->render(*meshMemoryLocations_, *textureHolder_, *materialHolder_, sampler_);
		return mainFramebuffer_->submitTo(*graphicsQueue_);
	}
	return 0.0f;
}

void vkn::Renderer::updateCamera(gee::Camera& camera, const float aspectRatio)
{
	shaderCamera_.position = glm::vec4{camera.position_, 1.0f};
	shaderCamera_.viewProj = camera.viewProjMatrix(aspectRatio);
}

vkn::Framebuffer& vkn::Renderer::getFramebuffer()
{
	assert(mainFramebuffer_ && "The main framebuffer has not been created yet");
	return *mainFramebuffer_;
}

vkn::Framebuffer& vkn::Renderer::getFramebuffer(std::vector<vkn::ShaderEffect>& effects, const bool enableGui)
{
	if (!mainFramebuffer_)
	{
		for (const auto& effect : effects)
		{
			if (effect.isPostProcessEffect())
			{
				meshMemoryLocations_->get(std::hash<std::string>{}("custom gee quad"), gee::getQuadMesh());
				break;
			}
		}
		mainFramebuffer_ = std::make_unique<vkn::Framebuffer>(*gpu_, *device_, surface_, *cbPool_, effects, enableGui, guiInitInfo_);
		if (enableGui)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}
	}
	return *mainFramebuffer_;
}

vkn::Framebuffer vkn::Renderer::createFramebuffer(const glm::u32vec2& extent, std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount)
{
	return vkn::Framebuffer{ *gpu_, *device_, *cbPool_, effects, VkExtent2D{extent.x, extent.y}, frameCount};
}

void vkn::Renderer::resize(const glm::u32vec2& size)
{
	if (size.x != 0 || size.y != 0)
	{
		isWindowMinimized_ = false;
		std::vector<vkn::ShaderEffect> pixelPerfectEffect;
		pixelPerfectEffect.emplace_back("pixel perfect", "../assets/Shaders/pixelPerfect/vert.spv", "../assets/Shaders/pixelPerfect/frag.spv");
		pixelPerfectFramebuffer_ = std::make_unique<vkn::Framebuffer>(*gpu_, *device_, *cbPool_, pixelPerfectEffect, VkExtent2D{ size.x, size.y }, 1);
		if (mainFramebuffer_)
		{
			mainFramebuffer_->resize(size);
		}
	}
	else
	{
		isWindowMinimized_ = true;
	}
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
}