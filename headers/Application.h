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
#include "vulkanContext.h"
#include "RenderTarget.h"
#include "Pipeline.h"

namespace gee
{
	class Application
	{
	public:
		Application(const std::string& name, const uint32_t width, const uint32_t height);
		~Application();
		bool isRunning();
		void setCameraPosition(const glm::vec3& position);
		void setSkybox(Drawable& skybox);
		void addDrawable(Drawable& drawable);
		void addCamera(const Camera& camera);
	private:
		gee::Window window_;
		std::unique_ptr<vkn::Context> context_;
		std::unique_ptr<vkn::RenderTarget> renderTarget_;
		std::unique_ptr<vkn::Pipeline> colorPipeline_;
		gee::EventDispatcher eventDispatcher_;
		std::unique_ptr<vkn::Renderer> renderer_;
		std::vector<std::reference_wrapper<gee::Drawable>> drawables_;
		std::vector<std::reference_wrapper<const Mesh>> meshes_;
		std::vector<std::reference_wrapper<gee::Texture>> textures_;
		Timer renderingtimer_;
		Camera camera_;

		void updateGui();
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);
		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };
		bool drawblesShouldBeSorted_{ false };
		glm::vec2 lastPos_{};
		size_t lastDrawableIndex_{-1u};
		std::optional<std::reference_wrapper<gee::Drawable>> activeDrawable_;
		std::optional<std::reference_wrapper<gee::Drawable>> skybox_{};
		gee::Timer cpuTimer_{ "Draw time" };
		float gpuTime_{};
		float cpuTime_{};
		
		void createContext();
		void createPipeline();

	};
}
