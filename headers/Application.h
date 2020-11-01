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
		void setCameraPosition(const glm::vec3& position);
		void addDrawable(Drawable& drawable);
		void addCamera(const Camera& camera);
	private: 
		gee::Window window_;
		gee::EventDispatcher eventDispatcher_;
		std::unique_ptr<vkn::Renderer> renderer_;
		std::vector<std::reference_wrapper<gee::Drawable>> drawables_;
		std::vector<std::reference_wrapper<gee::Drawable>> lightsDrawables_;
		std::vector<std::reference_wrapper<const Mesh>> meshes_;
		std::vector<std::reference_wrapper<gee::Texture>> textures_;
		Timer renderingtimer_;
		Camera camera_;

		//
		struct DrawableInfo
		{
			DrawableInfo(const std::string& name_, const std::string& meshName_, glm::vec3& position_, glm::vec4& color_, glm::vec3& rot, float& scaleFactor_, glm::vec3& size_);
			const std::string& name;
			const std::string& meshName;
			glm::vec3& position;
			glm::vec3& rotation;
			glm::vec4& color;
			glm::vec3& size;
			float& scaleFactor;

			void updateSize(float factor);
		};
		struct PointLightInfo
		{
			PointLightInfo(const std::string& name_, glm::vec3& pos, glm::vec3& amb, glm::vec3& diff, glm::vec3& spec, float& lin, float& quad);
			const std::string& name;
			glm::vec3& position;
			glm::vec3& ambient;
			glm::vec3& diffuse;
			glm::vec3& specular;
			float& linear;
			float& quadratic;
		};

		void updateGui();
		void displayDrawableInfo();
		void displayPointLightInfo();
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);
		void addDrawable(Drawable& drawable, std::vector<DrawableInfo>& infos);
		void addDrawable(Drawable& drawable, std::vector<PointLightInfo>& infos);
		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };
		glm::vec2 lastPos_{};


		std::vector<DrawableInfo> drawablesInfos;
		std::vector<PointLightInfo> pointLightInfos_;
	};
}
