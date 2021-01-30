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

struct ShaderCamera
{
	glm::vec4 position;
	glm::mat4 viewProj;
};
struct ShaderMaterial
{
	uint32_t diffuseTex;
	uint32_t normalTex;
	uint32_t specularTex;
};

using Mesh_t = std::reference_wrapper<const gee::Mesh>;
namespace std
{
	template<> struct hash<Mesh_t>
	{
		std::size_t operator()(const Mesh_t& mesh) const noexcept
		{
			return mesh.get().hash();
		}
	};

	template<> struct equal_to<Mesh_t>
	{
		bool operator()(const Mesh_t& lhs, const Mesh_t& rhs) const
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
		std::unique_ptr<vkn::RenderTarget> renderTarget_;
		std::unique_ptr<vkn::Pipeline> skyboxPipeline_;
		std::unique_ptr<vkn::Pipeline> colorPipeline_;
		gee::EventDispatcher eventDispatcher_;
		std::unique_ptr<vkn::Renderer> renderer_;
		std::vector<std::pair<Mesh_t, size_t>> geometryCount_;
		std::vector<std::reference_wrapper<gee::Drawable>> drawables_;
		std::vector<std::reference_wrapper<const gee::Texture>> textures_;
		std::unordered_map<Mesh_t, ShaderMaterial, std::hash<Mesh_t>, std::equal_to<Mesh_t>> materials_;
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
		std::unique_ptr<gee::Mesh> cubeMesh_;
		std::shared_ptr<gee::Texture> skyboxTexture_;

	};
}
