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

	eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
		{
			if (w != 0 && h != 0)
			{
				window_.resize();
				renderer_->resize();
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

void gee::Application::addDrawable(Drawable& drawable)
{
	auto drawbleResult = std::find_if(std::begin(drawables_), std::end(drawables_), [&](const auto& d) { return d.get().hash() == drawable.hash(); });
	if (drawbleResult == std::end(drawables_))
	{
		drawables_.emplace_back(std::ref(drawable));

		drawablesInfos.emplace_back(drawable.name, drawable.mesh.name(), drawable.position, drawable.color, drawable.rotation, drawable.scaleFactor, drawable.size);
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
	ImGui::ShowDemoWindow();
	displayDrawableInfo();
	camera_.imguiDisplay();
	ImGui::Render();
}

void gee::Application::displayDrawableInfo()
{
	std::vector<const char*> names;
	names.reserve(std::size(drawablesInfos));
	for (const auto& drawableInfo : drawablesInfos)
	{
		names.push_back(drawableInfo.name.c_str());
	}

	ImGui::Begin("Drawables");
	static int selected_drawable = 0;
	ImGui::Combo("Names", &selected_drawable, std::data(names), std::size(names));
	auto& drawable = drawablesInfos[selected_drawable];
	ImGui::SliderFloat3("position", &drawable.position.x, -50.0_m, 50.0_m, "%.1f");
	ImGui::ColorEdit4("color", &drawable.color.x);

	//the engine uses radians units in internal but degrees for display
	glm::vec3 rotationDeg = glm::degrees(drawable.rotation);
	ImGui::SliderFloat3("rotation", &rotationDeg.x, 0.0f, 360.0f, "%.1f");
	drawable.rotation = glm::radians(rotationDeg);
	
	auto lastScaleFactor = drawable.scaleFactor;
	ImGui::SliderFloat("scale factor", &drawable.scaleFactor, 0.0f, 10.0f, "%.1f");
	if (drawable.scaleFactor == 0.0f)
	{
		drawable.scaleFactor = lastScaleFactor;
	}
	drawable.updateSize(drawable.scaleFactor / lastScaleFactor);

	auto sizeStr = std::string{ "x = " } + std::to_string(drawable.size.x) + std::string{ " y = " } + std::to_string(drawable.size.y) + std::string{ " z = " } + std::to_string(drawable.size.z);
	ImGui::LabelText("size", sizeStr.c_str());
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
	if (renderingtimer_.ellapsedMs() >= 1000 / 60.0f)
	{
		renderer_->updateGui([&]() { updateGui(); });
		renderer_->updateCamera(camera_, window_.aspectRatio());

		/* for now the texture IS the material
		renderer_->bindMaterial(materials_);
		*/
		renderer_->draw(drawables_);

		renderingtimer_.reset();
	}

	return window_.isOpen();
}

gee::Application::DrawableInfo::DrawableInfo(const std::string& name_, const std::string& meshName_, glm::vec3& position_, glm::vec4& color_, glm::vec3& rot, float& scaleFactor_, glm::vec3& size_):
	name{name_}, meshName{meshName_},
	position{ position_ }, color{ color_ }, rotation{rot},
	scaleFactor{scaleFactor_}, size{size_}
{
}

void gee::Application::DrawableInfo::updateSize(float factor)
{
	size *= factor;
}
