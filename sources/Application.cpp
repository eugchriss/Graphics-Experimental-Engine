#include "../headers/Application.h"
#include  "../headers/Gpu.h"
#include <iostream>
#include <sstream>
#include <future>
#include <functional>
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"
#include "../headers/vulkanFrameGraph.h"
#include "../headers/vulkanContextBuilder.h"
#include "../headers/PipelineBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, eventDispatcher_{ window_.window() }, renderingtimer_{ "main Timer" }
{
	std::cout << "The application has been launched.\n";
	createContext();
	renderer_ = std::make_unique<vkn::Renderer>(*context_, window_);

	vkn::FrameGraph frameGraph{};
	auto colorAtt = frameGraph.addColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
	auto depthAtt = frameGraph.addDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT);
	frameGraph.setAttachmentColorDepthContent(colorAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
	frameGraph.setAttachmentColorDepthContent(depthAtt, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
	auto& colorPass = frameGraph.addPass();
	colorPass.addColorAttachment(colorAtt);
	colorPass.addDepthStencilAttachment(depthAtt);
	frameGraph.setPresentAttachment(colorAtt);

	renderTarget_ = std::make_unique<vkn::RenderTarget>(frameGraph.createRenderTarget(*context_, renderer_->swapchain()));
	createPipeline();



	window_.setTitle(name);
	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			if (w != 0 && h != 0)
			{
				window_.resize();
				firstMouseUse_ = true;
			}
			renderTarget_->resize(glm::u32vec2{ w, h });
		});
	eventDispatcher_.addMouseScrollCallback([&](double x, double y)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (!io.WantCaptureMouse)
			{
				onMouseScrollEvent(x, y);
			}
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
	skybox_ = std::make_optional(std::ref(skybox));
}

void gee::Application::addDrawable(Drawable& drawable)
{
	drawblesShouldBeSorted_ = true;
	auto drawbleResult = std::find_if(std::begin(drawables_), std::end(drawables_), [&](const auto& d) { return d.get().hash() == drawable.hash(); });
	if (drawbleResult == std::end(drawables_))
	{
		drawables_.emplace_back(std::ref(drawable));
	}
}

void gee::Application::addCamera(const Camera& camera)
{
}

void gee::Application::updateGui()
{
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
	camera_.imguiDisplay();
	ImGui::Begin("Loop time");
	ImGui::LabelText("cpu time", std::to_string(cpuTime_).c_str(), "0.3f");
	ImGui::LabelText("gpu time", std::to_string(gpuTime_).c_str(), "0.3f");
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
		double x, y;
		glfwGetCursorPos(window_.window(), &x, &y);
		//auto drawableIndex = renderer_->objectAt(drawables_, x, y);
		activeDrawable_ = std::nullopt;
		/*if (drawableIndex.has_value())
		{
			if (drawableIndex.value() != lastDrawableIndex_)
			{
				activeDrawable_.emplace(std::ref(drawables_[drawableIndex.value()].get()));
				lastDrawableIndex_ = drawableIndex.value();
			}
			else
			{
				lastDrawableIndex_ = -1;
			}
		}
		else
		{
			lastDrawableIndex_ = -1;
		}*/
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
	contextBuilder.addDeviceExtention("VK_KHR_swapchain");
	contextBuilder.addDeviceExtention("VK_EXT_descriptor_indexing");
	contextBuilder.addDeviceExtention("VK_KHR_maintenance3");
	contextBuilder.addQueueFlag(VK_QUEUE_GRAPHICS_BIT);
	contextBuilder.addQueueFlag(VK_QUEUE_TRANSFER_BIT);
	contextBuilder.setQueueCount(2);

	context_ = std::make_unique<vkn::Context>(std::move(contextBuilder.build(window_)));
}

void gee::Application::createPipeline()
{
	vkn::PipelineBuilder builder{ "../assets/shaders/vert.spv", "../assets/shaders/frag.spv" };
	builder.addAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	builder.addRaterizationStage(VK_POLYGON_MODE_FILL);
	builder.addDepthStage(VK_COMPARE_OP_LESS);
	builder.addColorBlendStage();
	builder.addMultisampleStage(VK_SAMPLE_COUNT_1_BIT);
	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	builder.renderpass = renderTarget_->renderpass->renderpass();
	builder.subpass = 0;
	colorPipeline_ = std::make_unique<vkn::Pipeline>(builder.get(*context_));
}

bool gee::Application::isRunning()
{
	renderer_->begin(*renderTarget_, VkRect2D{ {0, 0}, {window_.size().x, window_.size().y } });
	renderer_->usePipeline(*colorPipeline_);
	renderer_->draw();
	renderer_->end(*renderTarget_);
	return window_.isOpen();
}