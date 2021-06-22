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
#include "MaterialInstance.h"
#include "Renderer.h"
#include "Renderpass.h"
#include "ResourceHolder.h"
#include "sampler.h"
#include "Timer.h"
#include "Texture.h"
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
		Renderer renderer_;
		Renderpass renderpass_;
		ShaderTechnique phongTechnique_;
		gee::EventDispatcher eventDispatcher_;
		std::vector<DrawableRef> drawables_;
		Camera camera_;
		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };

		Texture floorTex_{ "../assets/textures/floor.jpg" };
		glm::vec2 lastPos_{};
		glm::mat4 mat_{ 1.0f };
		//functions only
	private:
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);
	};
}
