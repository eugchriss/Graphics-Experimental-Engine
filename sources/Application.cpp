#include <algorithm>
#include <iostream>
#include <future>
#include <numeric>
#include <sstream>

#include "../headers/Application.h"
#include "../headers/impl/vulkan/imgui_impl_glfw.h"
#include "../headers/impl/vulkan/imgui_impl_vulkan.h"
#include "../headers/impl/vulkan/vulkanContextBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : renderer_{ name, width, height }, eventDispatcher_{ renderer_.window_handle() }, phongTechnique_{ "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv" }
{
	std::cout << "The application has been launched.\n";
	RenderTarget depthTarget{ .size = {width, height},
								.format = D24_UNORM_S8_UINT,
								.usage = DEPTH,
								.loadOp = CLEAR,
								.storeOp = STORE};
	Pass colorPass{phongTechnique_};
	colorPass.add_screen_target();
	colorPass.add_depth_target(depthTarget);

	renderpass_.add_pass(colorPass);

	{
		eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
			{
				firstMouseUse_ = true;
				renderer_.resize();
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
}

gee::Application::~Application()
{
}

void gee::Application::setCameraPosition(const glm::vec3& position)
{
	camera_.position_ = position;
}

void gee::Application::draw(Drawable& drawable)
{
	auto result = std::find_if(std::begin(drawables_), std::end(drawables_), [&](const auto& drawableRef) { return ID<Drawable>::get(drawableRef.get()) == ID<Drawable>::get(drawable); });
	if (result == std::end(drawables_))
	{
		drawables_.emplace_back(std::ref(drawable));
	}
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
		auto windowSize = renderer_.window_size();
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

bool gee::Application::isRunning()
{
	auto viewProj = camera_.perspectiveProjection(renderer_.aspect_ratio());
	viewProj[1][1] *= -1;
	viewProj *= camera_.pointOfView();
	auto cube = gee::getCubeGeometry();
	renderer_.start_renderpass(renderpass_);
	renderer_.use_shader_technique(phongTechnique_);
	renderer_.update_shader_value(ShaderValue{ .name = "transform_matrices", .address = &mat_, .size = sizeof(glm::mat4) });
	renderer_.update_shader_value(ShaderTexture{ .name = "colors", .sampler = Sampler{}, .texture = floorTex_});
	renderer_.push_shader_constant(ShaderValue{ .name = "camera", .address = &viewProj, .size = sizeof(viewProj)});
	renderer_.draw(cube);
	return renderer_.render();
}