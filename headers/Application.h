#pragma once
#include <memory>
#include <utility>
#include <unordered_map>
#include <functional>
#include <vector>
#include "Window.h"
#include "Swapchain.h"
#include "Drawable.h"
#include "Timer.h"
#include "Camera.h"
#include "EventDispatcher.h"
#include "vulkanContext.h"
#include "VulkanImGuiContext.h"
#include "RenderTarget.h"
#include "CommandPool.h"

struct ShaderCamera
{
	glm::vec4 position;
	glm::mat4 viewProj;
};
using MeshRef = std::reference_wrapper<const gee::Mesh>;
using DrawableRef = std::reference_wrapper<gee::Drawable>;

namespace std
{
	template<>
	struct hash<const MeshRef>
	{
		size_t operator()(const MeshRef& meshRef) const
		{
			return meshRef.get().hash();
		}
	};

	template<>
	struct hash<MeshRef>
	{
		size_t operator()(const MeshRef& meshRef) const
		{
			return meshRef.get().hash();
		}
	};
	template<>
	struct equal_to<MeshRef>
	{
		bool operator()(const MeshRef& lhs, const MeshRef& rhs) const
		{
			return lhs.get().hash() == rhs.get().hash();
		}
	};
}
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
		std::unique_ptr<vkn::Swapchain> swapchain_;
		std::unique_ptr<vkn::CommandPool> commandPool_;
		std::unique_ptr<vkn::Renderpass> renderpass_;
		std::unique_ptr<vkn::Pipeline> skyboxPipeline_;
		std::unique_ptr<vkn::Pipeline> colorPipeline_;
		std::unique_ptr<vkn::Pipeline> gammaCorrectionPipeline_;
		std::unique_ptr<vkn::RenderTarget> pixelPerfectRenderTarget_;
		std::unique_ptr<vkn::Pipeline> pixelPerfectPipeline_;
		gee::EventDispatcher eventDispatcher_;
		std::unordered_map<MeshRef, std::vector<DrawableRef>, std::hash<MeshRef>, std::equal_to<MeshRef>> drawablesGeometries_;
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
		bool useGammaCorrection_{ true };
		bool useHdr_{ true };
		float exposure_{ 1.0f };

		glm::vec2 lastPos_{};
		void createContext();
		void createPipeline();
		void getTransforms();
		void initPixelPerfect();
		std::unique_ptr<gee::Mesh> cubeMesh_;
		std::unique_ptr<gee::Mesh> quadMesh_;
		std::shared_ptr<gee::Texture> skyboxTexture_;
		std::unique_ptr<vkn::ImGuiContext> imguiContext_;
		std::optional<size_t> lastDrawableIndex_{};
		std::optional<std::reference_wrapper<gee::Drawable>> activeDrawable_;
		gee::Timer renderingtimer_{ "rendering" };
		gee::Timer cpuTimer_{ "cpu" };
		float cpuTime_;
		float gpuTime_;
	};
}
