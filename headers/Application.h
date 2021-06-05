#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Camera.h"
#include "impl/vulkan/CommandPool.h"
#include "Drawable.h"
#include "EventDispatcher.h"
#include "MaterialInstance.h"
#include "Renderpass.h"
#include "ResourceHolder.h"
#include "Timer.h"
#include "Window.h"

#include "impl/vulkan/Material.h"
#include "impl/vulkan/RenderTarget.h"
#include "impl/vulkan/Swapchain.h"
#include "impl/vulkan/vulkanContext.h"
#include "impl/vulkan/VulkanImGuiContext.h"

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
		void setSkybox(Drawable& skybox);
		void draw(Drawable& drawable);
		void addCamera(const Camera& camera);
		void start_renderpass(const Renderpass& rp);
		void use_shader_technique(const ShaderTechnique& technique);
	private:

		gee::Window window_;
		std::unique_ptr<vkn::Context> context_;
		std::unique_ptr<vkn::Swapchain> swapchain_;
		std::unique_ptr<vkn::CommandPool> commandPool_;
		std::unique_ptr<vkn::Renderpass> colorRenderpass_;
		std::unique_ptr<vkn::Pipeline> skyboxPipeline_;
		std::unique_ptr<vkn::Pipeline> colorPipeline_;
		std::unique_ptr<vkn::Pipeline> gammaCorrectionPipeline_;
		std::unique_ptr<vkn::RenderTarget> pixelPerfectRenderTarget_;
		std::unique_ptr<vkn::Pipeline> pixelPerfectPipeline_;
		gee::EventDispatcher eventDispatcher_;
		std::vector<DrawableRef> drawables_;
		std::unordered_map<std::string, vkn::RenderTarget> renderTargets_;
		Camera camera_;
		std::vector<glm::mat4> modelMatrices_;
		std::vector<glm::mat4> normalMatrices_;
		std::vector<gee::ShaderPointLight> pointLights_;
		//Mouse buttons
		bool leftButtonPressed_{ false };
		bool rightButtonPressed_{ false };
		bool firstMouseUse_{ true };
		bool drawblesShouldBeSorted_{ false };
		bool useGammaCorrection_{ true };
		bool useHdr_{ true };
		float exposure_{ 1.0f };

		glm::vec2 lastPos_{};
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

		std::unique_ptr<vkn::TextureMemoryHolder> textureMemoryHolder_;

		using GeometryHolder = ResourceHolder<gee::GeometryFactory, gee::Geometry>;
		std::unique_ptr<GeometryHolder> geometryHolder_;
		std::unique_ptr<vkn::GeometryMemoryHolder> geometryMemoryHolder_;

		std::unordered_map<ID<Material>::Type, vkn::Material> materials_;
		std::unordered_map<ID<Material>::Type, std::vector<MaterialInstanceRef>> materialBatches_;

		std::shared_ptr<vkn::CommandBufferRef> lastRecordedDrawsCommandBuffer_;
		//functions only
	private:
		void updateGui();
		void onMouseMoveEvent(double x, double y);
		void onMouseScrollEvent(double x, double y);
		void onMouseButtonEvent(uint32_t button, uint32_t action, uint32_t mods);

		void createContext();
		void createPipeline();
		void getTransforms();
		void initPixelPerfect();
		void create_renderpass(const uint32_t width, const uint32_t height);
		void batch_materials();
		void batch_material_instances();
	};
}
