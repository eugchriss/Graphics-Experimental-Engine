#include "../headers/Application.h"
#include  "../headers/Gpu.h"
#include <iostream>
#include <sstream>
#include <future>
#include <functional>
#include "../headers/imgui_impl_glfw.h"
#include "../headers/imgui_impl_vulkan.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, eventDispatcher_{ window_.window() }, renderingtimer_{ "main Timer" }
{
	std::cout << "The application has been launched.\n";
	renderer_ = std::make_unique<vkn::Renderer>(window_);
	std::ostringstream os;
	renderer_->getGpuInfo(os);

	window_.setTitle(name + ": " + os.str());
	std::vector<vkn::ShaderEffect> effects;
	effects.emplace_back("skybox technique", "../assets/Shaders/skybox/vert.spv", "../assets/Shaders/skybox/frag.spv");
	effects.emplace_back("forward rendering", "../assets/shaders/vert.spv", "../assets/shaders/frag.spv");
	mainFb_ = renderer_->getFramebuffer(effects);
	mainFb_->setViewport(0.0f, 0.0f, static_cast<float>(window_.size().x), static_cast<float>(window_.size().y));

	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			if (w != 0 && h != 0)
			{
				window_.resize();
				mainFb_->resize(glm::u32vec2{w, h});
				firstMouseUse_ = true;
			}
			else
			{
				renderer_->setWindowMinimized(true);
			}
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
	if (drawable.hasLightComponent())
	{
		auto result = std::find_if(std::begin(lights_), std::end(lights_), [&](const auto& d) { return d.get().hash() == drawable.hash(); });
		if (result == std::end(lights_))
		{
			lights_.emplace_back(std::ref(drawable));
		}
	}
	else
	{
		auto drawbleResult = std::find_if(std::begin(drawables_), std::end(drawables_), [&](const auto& d) { return d.get().hash() == drawable.hash(); });
		if (drawbleResult == std::end(drawables_))
		{
			drawables_.emplace_back(std::ref(drawable));
		}
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
	if (std::size(drawables_) > 0)
	{
		displayDrawableInfo();
	}

	if (std::size(lights_) > 0)
	{
		displayPointLightInfo();
	}
	camera_.imguiDisplay();
}

void gee::Application::displayDrawableInfo()
{
	std::vector<const char*> names;
	names.reserve(std::size(drawables_));
	for (const auto& drawable: drawables_)
	{
		names.push_back(drawable.get().name.c_str());
	}

	ImGui::Begin("Drawables");
	static int selected_drawable = 0;
	ImGui::Combo("Names", &selected_drawable, std::data(names), std::size(names));
	auto& drawable = drawables_[selected_drawable].get();
	auto position = drawable.getPosition();
	ImGui::SliderFloat3("position", &position.x, -50.0_m, 50.0_m, "%.1f");
	ImGui::ColorEdit4("color", &drawable.color.x);

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
	ImGui::End();

	drawable.setPosition(position);
	drawable.setRotation(rotation);
	if (drawable.scaleFactor == 0.0f)
	{
		drawable.scaleFactor = lastScaleFactor;
	}
	drawable.setSize(drawable.getSize() * drawable.scaleFactor / lastScaleFactor);
}

void gee::Application::displayPointLightInfo()
{
	std::vector<const char*> names;
	names.reserve(std::size(lights_));
	for (const auto& light : lights_)
	{
		names.push_back(light.get().name.c_str());
	}

	ImGui::Begin("PointLights");
	static int selected_pointLight = 0;
	ImGui::Combo("Names", &selected_pointLight, std::data(names), std::size(names));
	auto& light = lights_[selected_pointLight].get().light();

	ImGui::SliderFloat3("position", &light.position.x, -50.0_m, 50.0_m, "%.1f");
	ImGui::ColorEdit3("ambient", &light.ambient.x);
	ImGui::ColorEdit3("diffuse", &light.diffuse.x);
	ImGui::SliderFloat("specular", &light.specular.x, 0.0f, 256.0f , "%f");
	light.specular.y = light.specular.x;
	light.specular.z = light.specular.x;
	ImGui::SliderFloat("linear", &light.linear, 0.0014f, 0.7, "%.4f");
	ImGui::SliderFloat("quadratic", &light.quadratic, 0.0000007f, 1.8, "%.7f");
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
		auto drawableIndex = renderer_->objectAt(drawables_, x, y);
		if (drawableIndex.has_value())
		{
			//activeDrawable_.emplace(drawables_[drawableIndex.value()].get());
			std::cout << activeDrawable_->name << "\n";
		}
		else
		{
			std::cout << "no active drawable" << "\n";
			activeDrawable_ = std::nullopt;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		leftButtonPressed_ = false;
		firstMouseUse_ = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		rightButtonPressed_ = false;
		activeDrawable_ = std::nullopt;
	}
}

bool gee::Application::isRunning()
{
	if (renderingtimer_.ellapsedMs() >= 1000 / 60.0f)
	{
		renderer_->updateCamera(camera_, window_.aspectRatio());
		updateGui();

		gee::Timer t{ "Draw time" };
		std::vector<std::reference_wrapper<gee::Drawable>> v;
		if (skybox_.has_value())
		{
			v.emplace_back(skybox_.value());
			renderer_->render(*mainFb_, "skybox technique", v);
		}
		renderer_->render(*mainFb_, "forward rendering", drawables_);
		renderer_->draw(*mainFb_);
		std::cout << "draw time : " << t.ellapsedMs() << "ms\n";
		renderingtimer_.reset();
	}

	return window_.isOpen();
}