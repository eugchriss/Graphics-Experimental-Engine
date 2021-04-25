#include "../headers/Application.h"
#include <iostream>
#include <sstream>
#include <future>
#include <numeric>
#include <algorithm>
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/vulkanContextBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, eventDispatcher_{ window_.window() }
{
	window_.setTitle(name);
	std::cout << "The application has been launched.\n";

	createContext();
	textureMemoryHolder_ = std::make_unique<vkn::TextureMemoryHolder>(vkn::TextureImageFactory{ *context_ });
	geometryHolder_ = std::make_unique<GeometryHolder>(gee::GeometryFactory{});
	geometryMemoryHolder_ = std::make_unique<vkn::GeometryMemoryHolder>(vkn::GeometryMemoryLocationFactory{ *context_ });

	commandPool_ = std::make_unique<vkn::CommandPool>(*context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	swapchain_ = std::make_unique<vkn::Swapchain>(*context_);

	create_renderpass(width, height);

	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			firstMouseUse_ = true;
			window_.resize();
			if (window_.isVisible())
			{
				context_->device->idle();
				swapchain_->resize(VkExtent2D{ w, h });
				colorRenderpass_->resize(glm::u32vec2{ w, h });
			}
		});
	eventDispatcher_.addMouseScrollCallback([&](double x, double y)
		{
			onMouseScrollEvent(x, y);
		});
	eventDispatcher_.addMouseButtonCallback([&](uint32_t button, uint32_t action, uint32_t mods)
		{
			onMouseButtonEvent(button, action, mods);
			firstMouseUse_ = true;

		});
	eventDispatcher_.addMouseMoveCallback([&](double x, double y)
		{
			onMouseMoveEvent(x, y);
		});
}

gee::Application::~Application()
{
	context_->device->idle();
}

void gee::Application::setCameraPosition(const glm::vec3& position)
{
	camera_.position_ = position;
}

void gee::Application::setSkybox(Drawable& skybox)
{
}

void gee::Application::add_drawable(Drawable& drawable)
{
	drawables_.emplace_back(std::ref(drawable));
}

void gee::Application::add_drawable_without_duplicate(Drawable& drawable)
{
	auto result = std::find_if(std::begin(drawables_), std::end(drawables_), [&](const auto& drawableRef) { return drawableRef.get().name == drawable.name; });
	if (result == std::end(drawables_))
	{
		drawables_.emplace_back(std::ref(drawable));
	}
}

void gee::Application::addCamera(const Camera& camera)
{
}

vkn::Material& gee::Application::get_material(const std::string& name, const std::string vertexPath, const std::string& fragmentPath)
{
	return materials_.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(*context_, vertexPath, fragmentPath)).first->second;
}

const gee::Geometry& gee::Application::get_geometry(const std::string& name, gee::Mesh& mesh)
{
	return geometryHolder_->get(name, mesh);
}

const gee::Texture& gee::Application::load_texture(const std::string& name, const std::string& path, const gee::Texture::ColorSpace colorSpace)
{
	return textures_.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(name, path, colorSpace)).first->second;
}

gee::MaterialInstance& gee::Application::get_materialInstance(vkn::Material& material)
{
	return materialBatches_[material].emplace_back(material);
}

void gee::Application::updateGui()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (activeDrawable_)
	{
		auto& drawable = activeDrawable_.value().get();
		ImGui::Begin(drawable.name.c_str());
		auto position = drawable.getPosition();
		ImGui::SliderFloat3("position", &position.x, -500.0_m, 500.0_m, "%.1f");


		//the engine uses radians units in internal but degrees for display
		auto rotation = drawable.getRotation();
		glm::vec3 rotationDeg = glm::degrees(rotation);
		ImGui::SliderFloat3("rotation", &rotationDeg.x, 0.0f, 360.0f, "%.1f");
		rotation = glm::radians(rotationDeg);

		auto lastScaleFactor = drawable.scaleFactor;
		ImGui::SliderFloat("scale factor", &drawable.scaleFactor, 0.0f, 10.0f, "%.1f");

		auto size = drawable.getSize();
		auto sizeStr = std::string{ "x = " } +std::to_string(size.x) + std::string{ " y = " } +std::to_string(size.y) + std::string{ " z = " } +std::to_string(size.z);
		ImGui::LabelText("size", sizeStr.c_str());

		ImGui::End();

		drawable.setPosition(position);
		drawable.setRotation(rotation);
		if (drawable.scaleFactor == 0.0f)
		{
			drawable.scaleFactor = lastScaleFactor;
		}
		drawable.setSize(drawable.getSize() * drawable.scaleFactor / lastScaleFactor);
	}

	ImGui::Begin("Loop time");
	ImGui::LabelText("cpu time (ms)", std::to_string(cpuTime_).c_str(), "0.3f");
	ImGui::LabelText("gpu time (ms)", std::to_string(gpuTime_).c_str(), "0.3f");
	ImGui::End();

	ImGui::Begin("Postprocess effects");
	ImGui::Checkbox("use hdr", &useHdr_);
	ImGui::SliderFloat("exposure", &exposure_, 0.1, 5.0f, "%.1f");
	ImGui::Checkbox("use gamma correction", &useGammaCorrection_);
	ImGui::End();
}

void gee::Application::onMouseMoveEvent(double x, double y)
{
	if (leftButtonPressed_)
	{
		if (firstMouseUse_)
		{
			lastPos_.x = x;
			lastPos_.y = y;
			firstMouseUse_ = false;
		}
		auto windowSize = window_.size();
		auto yaw = (x - lastPos_.x) * 2.0f * 360.0f / windowSize.x;
		auto pitch = (lastPos_.y - y) * 2.0f * 360.0f / windowSize.y;
		lastPos_ = glm::vec2{ x, y };

		camera_.rotate(pitch, yaw);
	}
	if (rightButtonPressed_)
	{

	}
}

void gee::Application::onMouseScrollEvent(double x, double y)
{
	if (y > 0)
	{
		camera_.move(FORWARD);
	}
	else if (y < 0)
	{
		camera_.move(BACKWARD);
	}
}

void gee::Application::onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		leftButtonPressed_ = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		rightButtonPressed_ = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		leftButtonPressed_ = false;
		firstMouseUse_ = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rightButtonPressed_ = false;
	}
}

void gee::Application::createContext()
{

	VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	vkn::ContextBuilder contextBuilder{ severityFlags, messageTypeFlags };
	contextBuilder.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	contextBuilder.addInstanceLayer("VK_LAYER_LUNARG_monitor");
	contextBuilder.addDeviceExtention("VK_KHR_swapchain");
	contextBuilder.addDeviceExtention("VK_EXT_descriptor_indexing");
	contextBuilder.addDeviceExtention("VK_KHR_maintenance3");
	contextBuilder.addDeviceExtention("VK_KHR_push_descriptor");
	contextBuilder.addQueueFlag(VK_QUEUE_GRAPHICS_BIT);
	contextBuilder.addQueueFlag(VK_QUEUE_TRANSFER_BIT);
	contextBuilder.setQueueCount(2);

	context_ = std::make_unique<vkn::Context>(std::move(contextBuilder.build(window_)));
}

void gee::Application::getTransforms()
{
	modelMatrices_.clear();
	normalMatrices_.clear();
	pointLights_.clear();
	/*for (auto& [mesh, drawableRefs] : drawablesGeometries_)
	{
		for (const auto& drawableRef : drawableRefs)
		{
			auto& drawable = drawableRef.get();
			modelMatrices_.emplace_back(drawable.getTransform());
			normalMatrices_.emplace_back(drawable.getNormalMatrix());
			if (drawable.hasLightComponent())
			{
				auto& light = drawable.light();

				auto& shaderLight = pointLights_.emplace_back(gee::ShaderPointLight{});
				shaderLight.ambient = glm::vec4{ light.ambient, 1.0f };
				shaderLight.diffuse = glm::vec4{ light.diffuse, 1.0 };
				shaderLight.specular = glm::vec4{ light.specular, 1.0f };
				shaderLight.position = glm::vec4{ light.position, 1.0f };
				shaderLight.linear = light.linear;
				shaderLight.quadratic = light.quadratic;
			}
		}
	}*/
}

void gee::Application::create_renderpass(const uint32_t width, const uint32_t height)
{
	VkExtent2D size{ width, height };
	auto& colorTargets = swapchain_->renderTargets();
	for (auto& target : colorTargets)
	{
		target.loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
		target.storeOperation = VK_ATTACHMENT_STORE_OP_STORE;
	}

	if (renderTargets_.find("depthTarget") == std::end(renderTargets_))
	{
		renderTargets_.emplace(std::piecewise_construct,
			std::forward_as_tuple("depthTarget"),
			std::forward_as_tuple(*context_, VK_FORMAT_D24_UNORM_S8_UINT, size, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
		auto& depthTarget = renderTargets_.find("depthTarget")->second;
		depthTarget.loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}

	vkn::Pass colorPass;
	for (auto& colorTarget : colorTargets)
	{
		colorPass.usesColorTarget(colorTarget);
	}
	colorPass.usesDepthStencilTarget(renderTargets_.find("depthTarget")->second);
	std::vector<vkn::Pass> passes;
	passes.emplace_back(std::move(colorPass));
	colorRenderpass_ = std::make_unique<vkn::Renderpass>(*context_, size, std::move(passes));
}

void gee::Application::batch_material_instances()
{
	for (auto& [materialRef, materialInstances] : materialBatches_)
	{
		for (auto& materialInstance : materialInstances)
		{
			materialInstance.reset_transforms();
		}
	}
	for (auto& drawableRef : drawables_)
	{
		auto& drawable = drawableRef.get();
		drawable.materialInstance.get().add_geometry(drawable.geometry, drawable.getTransform());
	}
}

bool gee::Application::isRunning()
{
	if ((renderingtimer_.ellapsedMs() >= 100 / 6.0f))
	{
		if (window_.isVisible())
		{
			auto& cb = commandPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			cb.begin();
			colorRenderpass_->begin(cb, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
			batch_material_instances();
			for (auto& [materialRef, materialInstances] : materialBatches_)
			{
				materialRef.get().bind((*colorRenderpass_)());
				materialRef.get().draw(*geometryMemoryHolder_, *textureMemoryHolder_, cb, camera_.get_shader_info(window_.aspectRatio()), materialInstances);
			}
			colorRenderpass_->end(cb);
			cb.end();
			context_->graphicsQueue->submit(cb);
			swapchain_->swapBuffers();
			context_->graphicsQueue->present(cb, *swapchain_);
		}
	}
	return window_.isOpen();
}