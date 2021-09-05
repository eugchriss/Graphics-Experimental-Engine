#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Camera.h"
#include "impl/vulkan/CommandPool.h"
#include "Drawable.h"
#include "EventDispatcher.h"
#include "Material.h"
#include "Renderer.h"
#include "Renderpass.h"
#include "ResourceHolder.h"
#include "sampler.h"
#include "Timer.h"
#include "Texture.h"
#include "VulkanContext.h"
#include "Window.h"


struct ShaderCamera
{
	glm::vec4 position;
	glm::mat4 viewProj;
};

namespace gee
{
	class Application
	{
	public:
		Application(const std::string& name, const uint32_t width, const uint32_t height);
		~Application();
		bool isRunning();
		void setCameraPosition(const glm::vec3& position);
		void draw(Drawable& drawable);
	private:
		gee::Window window_;
		VulkanContext vulkanContext_;
		Renderer renderer_;
		Renderpass renderpass_;
		ShaderTechnique phongTechnique_;
		gee::EventDispatcher eventDispatcher_;
		std::vector<DrawableRef> drawables_;
		std::vector<glm::mat4> drawablesTransforms_;
		std::vector<std::reference_wrapper<const Material>> materials_;
		std::unordered_map<ID<Material>::Type, std::vector<size_t>> materialsDrawables_;
		Camera camera_;
		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };


		glm::vec2 lastPos_{};
		//functions only
	private:
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);
		void sort_materials_drawables();
		std::vector<gee::ShaderArrayTexture> get_arrayTextures();
	};
}
