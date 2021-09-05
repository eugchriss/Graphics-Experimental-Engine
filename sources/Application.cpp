#include <algorithm>
#include <iostream>
#include <future>
#include <numeric>
#include <sstream>

#include "../headers/Application.h"
#include "../libs/imgui/backends/imgui_impl_glfw.h"
#include "../libs/imgui/backends/imgui_impl_vulkan.h"
#include "../headers/impl/vulkan/vulkanContextBuilder.h"

gee::Application::Application(const std::string& name, const uint32_t width, const uint32_t height) : window_{ name, width, height }, vulkanContext_{ window_ }, renderer_{ vulkanContext_, window_ }, eventDispatcher_{window_.window()}, phongTechnique_{ "../assets/shaders/triangleShader.spv", "../assets/shaders/greenColoredShader.spv" }
{
	std::cout << "The application has been launched.\n";
	phongTechnique_.set_dynamic_alignments(0, { 1024 * sizeof(glm::mat4) } );

	RenderTarget depthTarget{ .size = {width, height},
								.format = D24_UNORM_S8_UINT,
								.usage = DEPTH,
								.loadOp = CLEAR,
								.storeOp = STORE};
	Pass colorPass{phongTechnique_};
	colorPass.add_screen_target();
	colorPass.add_depth_target(depthTarget);

	renderpass_.add_pass(colorPass);
	renderpass_.add_gui_pass();

	{
		eventDispatcher_.addWindowResizeCallback([&](const uint32_t w, const uint32_t h)
			{
				firstMouseUse_ = true;
				renderer_.resize();
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
					firstMouseUse_ = true;
				}

			});
		eventDispatcher_.addMouseMoveCallback([&](double x, double y)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (!io.WantCaptureMouse)
				{
					onMouseMoveEvent(x, y);
				}
			});
	}
	drawablesTransforms_.resize(1024*15);
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
	auto result = std::find(std::begin(drawables_), std::end(drawables_), drawable);
	if (result == std::end(drawables_))
	{
		drawables_.emplace_back(drawable);
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

void gee::Application::sort_materials_drawables()
{
	for (auto i = 0u; i < std::size(drawables_); ++i)
	{
		auto& material = drawables_[i].get().material();
		auto materialResult = std::find_if(std::begin(materials_), std::end(materials_), [&](auto& mat) {return mat == material; });
		if (materialResult == std::end(materials_))
		{
			materials_.emplace_back(std::cref(material));
		}
		materialsDrawables_[ID<Material>::get(material)].emplace_back(i);
	}
}

std::vector<gee::ShaderArrayTexture> gee::Application::get_arrayTextures()
{
	gee::Sampler sampler{};
	gee::ShaderArrayTexture colorsArrayTexture{ .name = "colors", .sampler = sampler};
	gee::ShaderArrayTexture normalsArrayTexture{ .name = "normals", .sampler = sampler};

	//drawablesTransforms_.reserve(std::size(drawables_));

	uint32_t materialIndex = 0u;
	for (const auto& [materialID, drawablesIndices] : materialsDrawables_)
	{
		const auto result = std::find_if(std::begin(materials_), std::end(materials_), [&](const auto& mat) {return ID<Material>::get(mat.get()) == materialID; });
		assert(result != std::end(materials_) && "material ID not matching any actual material");

		const auto& properties = result->get().properties();
		auto colorPropertyExist = properties.find(MaterialProperty::COLOR);
		colorPropertyExist != std::end(properties) ? colorsArrayTexture.add(colorPropertyExist->second) : colorsArrayTexture.add_empty_texture();

		auto normalPropertyExist = properties.find(MaterialProperty::NORMAL);
		normalPropertyExist != std::end(properties) ? normalsArrayTexture.add(normalPropertyExist->second) : normalsArrayTexture.add_empty_texture();

		auto drawableIndex = 0u;
		for (const auto index : drawablesIndices)
		{
			drawablesTransforms_[materialIndex * 1024 + drawableIndex] = drawables_[index].get().getTransform();
			++drawableIndex;
		}
		++materialIndex;
	}
	return { colorsArrayTexture, normalsArrayTexture };
}

bool gee::Application::isRunning()
{
	auto viewProj = camera_.perspectiveProjection(window_.aspectRatio());
	viewProj[1][1] *= -1;
	viewProj *= camera_.pointOfView();
	renderer_.start_renderpass(renderpass_);
	renderer_.use_shader_technique(phongTechnique_);

	sort_materials_drawables();
	auto arrayTextures = get_arrayTextures();
	renderer_.update_shader_value(ShaderValue{ "transform_matrices", drawablesTransforms_ });
	for (const auto& arrayTexture : arrayTextures)
	{
		renderer_.update_shader_value(arrayTexture);
	}

	uint32_t materialIndex = 0u;
	for (const auto& [materialID, drawableIndices] : materialsDrawables_)
	{
		renderer_.new_batch();
		renderer_.push_shader_constant(ShaderValue{"camera", viewProj});
		renderer_.push_shader_constant(ShaderValue{"materialIndex", materialIndex});
		for (const auto index : drawableIndices)
		{
			renderer_.draw(drawables_[index].get().geometry());
		}
		++materialIndex;
	}
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("test");
		ImGui::LabelText("size", "label");
		ImGui::End();
	}
	renderer_.render_gui();

	drawables_.clear();
	materialsDrawables_.clear();
	//drawablesTransforms_.clear();
	renderer_.render();
	return window_.isOpen();
}