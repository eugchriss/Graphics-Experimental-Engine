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
#include "VulkanImGuiContext.h"
#include "RenderTarget.h"
#include "Pipeline.h"
#include "QueryPool.h"
#include "Query.h"

struct ShaderCamera
{
	glm::vec4 position;
	glm::mat4 viewProj;
};
using Mesh_t = std::reference_wrapper<const gee::Mesh>;
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
		std::unique_ptr<vkn::Pipeline> skyboxPipeline_;
		std::unique_ptr<vkn::Pipeline> colorPipeline_;
		std::unique_ptr<vkn::Pipeline> gammaCorrectionPipeline_;
		std::unique_ptr<vkn::RenderTarget> pixelPerfectRenderTarget_;
		std::unique_ptr<vkn::Pipeline> pixelPerfectPipeline_;
		gee::EventDispatcher eventDispatcher_;
		std::unique_ptr<vkn::Renderer> renderer_;
		std::vector<std::pair<Mesh_t, size_t>> geometryCount_;
		std::vector<std::reference_wrapper<gee::Drawable>> drawables_;
		Camera camera_;
		std::vector<glm::mat4> modelMatrices_;
		std::vector<glm::mat4> normalMatrices_;
		std::vector<gee::ShaderPointLight> pointLights_;
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
		void createContext();
		void createPipeline();
		void getTransforms();
		void initPixelPerfect();
		std::unique_ptr<gee::Mesh> cubeMesh_;
		std::unique_ptr<gee::Mesh> quadMesh_;
		std::shared_ptr<gee::Texture> skyboxTexture_;
		std::unique_ptr<vkn::ImGuiContext> imguiContext_;
		size_t lastDrawableIndex_{ 0u };
		std::optional<std::reference_wrapper<gee::Drawable>> activeDrawable_;
		gee::Timer renderingtimer_{ "rendering" };
		gee::Timer cpuTimer_{ "cpu" };
		float cpuTime_;
		float gpuTime_;
		std::unique_ptr<vkn::QueryPool> queryPool_;
	};
}
