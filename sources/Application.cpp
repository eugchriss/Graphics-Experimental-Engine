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
	commandPool_ = std::make_unique<vkn::CommandPool>(*context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	swapchain_ = std::make_unique<vkn::Swapchain>(*context_);
	auto& colorTargets = swapchain_->renderTargets();
	for (auto& target : colorTargets)
	{
		target.loadOperation = VK_ATTACHMENT_LOAD_OP_CLEAR;
		target.storeOperation = VK_ATTACHMENT_STORE_OP_STORE;
	}

	vkn::Pass colorPass{ "../assets/shaders/vert.spv", "../assets/shaders/frag.spv" };
	colorPass.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	colorPass.addRaterizationStage(VK_POLYGON_MODE_FILL);
	//colorPass.addDepthStage(VK_COMPARE_OP_LESS);
	colorPass.addColorBlendStage();
	colorPass.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	colorPass.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	colorPass.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	for (auto& colorTarget : colorTargets)
	{
		colorPass.usesColorTarget(colorTarget);
	}

	std::vector<vkn::Pass> passes;
	passes.emplace_back(std::move(colorPass));

	renderpass_ = std::make_unique<vkn::Renderpass>(*context_, VkExtent2D{width, height}, std::move(passes));

	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			window_.resize();
			if (w != 0 && h != 0)
			{
				context_->device->idle();
				firstMouseUse_ = true;
				swapchain_->resize(VkExtent2D{ w, h });
				renderpass_->resize(glm::u32vec2{ w, h });
			}			
		});
	eventDispatcher_.addMouseScrollCallback([&](double x, double y)
		{
		/*	ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				onMouseScrollEvent(x, y);
			}*/
		});
	eventDispatcher_.addMouseButtonCallback([&](uint32_t button, uint32_t action, uint32_t mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				onMouseButtonEvent(button, action, mods);
			}
			else
			{
				firstMouseUse_ = true;
			}
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

void gee::Application::addDrawable(Drawable& drawable)
{
	auto& drawables = drawablesGeometries_[std::ref(drawable.mesh)];
	auto result = std::find_if(std::begin(drawables), std::end(drawables), [&](const auto& drawableRef) { return drawableRef.get().name == drawable.name; });
	if (result == std::end(drawables))
	{
		drawables.emplace_back(std::ref(drawable));
	}
}

void gee::Application::addCamera(const Camera& camera)
{
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

		auto color = drawable.getColor();
		ImGui::ColorEdit4("color", &color.x);

		//the engine uses radians units in internal but degrees for display
		auto rotation = drawable.getRotation();
		glm::vec3 rotationDeg = glm::degrees(rotation);
		ImGui::SliderFloat3("rotation", &rotationDeg.x, 0.0f, 360.0f, "%.1f");
		rotation = glm::radians(rotationDeg);

		auto lastScaleFactor = drawable.scaleFactor;
		ImGui::SliderFloat("scale factor", &drawable.scaleFactor, 0.0f, 10.0f, "%.1f");

		auto size = drawable.getSize();
		auto sizeStr = std::string{ "x = " } + std::to_string(size.x) + std::string{ " y = " } + std::to_string(size.y) + std::string{ " z = " } + std::to_string(size.z);
		ImGui::LabelText("size", sizeStr.c_str());
		if (drawable.hasLightComponent())
		{
			auto& light = drawable.light();
			ImGui::ColorEdit3("ambient", &light.ambient.x);
			ImGui::ColorEdit3("diffuse", &light.diffuse.x);
			ImGui::SliderFloat("specular", &light.specular.x, 0.0f, 256.0f, "%f");
			light.specular.y = light.specular.x;
			light.specular.z = light.specular.x;
			ImGui::SliderFloat("linear", &light.linear, 0.0014f, 0.7, "%.4f");
			ImGui::SliderFloat("quadratic", &light.quadratic, 0.0000007f, 1.8, "%.7f");
		}
		ImGui::End();

		drawable.setPosition(position);
		drawable.setRotation(rotation);
		drawable.setColor(color);
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
	for (auto& [mesh, drawableRefs] : drawablesGeometries_)
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
			renderpass_->begin(cb, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y} });
			renderpass_->draw(cb, 3, 1, 0, 0);
			renderpass_->end(cb);
			cb.end();
			context_->graphicsQueue->submit(cb);
			swapchain_->swapBuffers();
			context_->graphicsQueue->present(cb, *swapchain_);
		}
	}
	return window_.isOpen();
}