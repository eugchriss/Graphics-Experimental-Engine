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
	effects.emplace_back("gamma correction", "../assets/Shaders/gamma correction/vert.spv", "../assets/Shaders/gamma correction/frag.spv", true);
	renderer_->getFramebuffer(effects);
	renderer_->setViewport(0.0f, 0.0f, static_cast<float>(window_.size().x), static_cast<float>(window_.size().y));

	renderer_->getFramebuffer().getEffect("gamma correction").setBooleanTweaking("gammaCorrection");
	renderer_->getFramebuffer().getEffect("gamma correction").setBooleanTweaking("hdr");
	renderer_->getFramebuffer().getEffect("gamma correction").setTweakingRange("exposure", 0.1f, 5.0f);
	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			if (w != 0 && h != 0)
			{
				window_.resize();
				firstMouseUse_ = true;
			}
			renderer_->resize(glm::u32vec2{ w, h });
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
	displayShaderTweakings(renderer_->getFramebuffer().shaderTweakings());

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
		auto drawableIndex = renderer_->objectAt(drawables_, x, y);
		activeDrawable_ = std::nullopt;
		if (drawableIndex.has_value())
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
	}
}

void gee::Application::displayShaderTweakings(std::vector<std::reference_wrapper<vkn::Shader::Tweaking>>& tweakings)
{
	ImGui::Begin("Shader Tweakings");
	for (auto& tweakingRef : tweakings)
	{
		auto& tweaking = tweakingRef.get();
		if (tweaking.dataType == vkn::Shader::GLSL_Type::BOOL)
		{
			bool value = tweaking.data;
			ImGui::Checkbox(tweaking.name.c_str(), &value);
			tweaking.data = value;
		}
		else if (tweaking.dataType == vkn::Shader::GLSL_Type::FLOAT)
		{
			ImGui::SliderFloat(tweaking.name.c_str(), &tweaking.data, tweaking.min, tweaking.max, "%.1f");
		}
	}
	ImGui::End();
}

bool gee::Application::isRunning()
{
	if ((renderingtimer_.ellapsedMs() >= 100 / 6.0f))
	{
		renderingtimer_.reset();
		cpuTimer_.reset();
		if (drawblesShouldBeSorted_)
		{
			std::sort(std::begin(drawables_), std::end(drawables_), [&](const auto& lhs, const auto& rhs) { return rhs.get().mesh.hash() < lhs.get().mesh.hash(); });
		}
		renderer_->updateCamera(camera_, window_.aspectRatio());
		updateGui();
		std::vector<std::reference_wrapper<gee::Drawable>> v;
		if (skybox_.has_value())
		{
			v.emplace_back(skybox_.value());
			renderer_->render("skybox technique", v);
		}
		renderer_->render("forward rendering", drawables_);
		gpuTime_ = renderer_->draw();
		cpuTime_ = cpuTimer_.ellapsedMs();
	}
	

	return window_.isOpen();
}