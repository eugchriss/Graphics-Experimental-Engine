#pragma once
#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>
#include "Window.h"
#include "VulkanRenderer.h"
#include "Drawable.h"
#include "Timer.h"
#include "Camera.h"
#include "EventDispatcher.h"

namespace gee
{
	class Application
	{
	public:
		Application::Application(const std::string& name, const uint32_t width, const uint32_t height);
		bool gee::Application::isRunning();
		void addDrawable(Drawable& drawable);
		void addCamera(const Camera& camera);

	private: 
		gee::Window window_;
		gee::EventDispatcher eventDispatcher_;
		std::unique_ptr<vkn::Renderer> renderer_;
		std::vector<std::reference_wrapper<gee::Drawable>> drawables_;
		std::vector<std::reference_wrapper<const Mesh>> meshes_;
		std::vector<std::reference_wrapper<gee::Texture>> textures_;
		Timer renderingtimer_;
		Camera camera_;

		void updateGui();
		void displayDrawableInfo();
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);

		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };
		glm::vec2 lastPos_{};


		//
		struct DrawableInfo
		{
			DrawableInfo(const std::string& name_, const std::string& meshName_, glm::vec3& position_, glm::vec4& color_, glm::vec3& rot);
			const std::string& name;
			const std::string& meshName;
			glm::vec3& position;
			glm::vec3& rotation;
			glm::vec4& color;
		};

		std::vector<DrawableInfo> drawablesInfos;
	};
}
