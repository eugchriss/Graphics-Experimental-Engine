#pragma once
#include <ostream>
#include <unordered_map>
#include <functional>
#include <utility>
#include <array>
#include <string>

#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "window.h"
#include "Instance.h"
#include "gpu.h"
#include "DebugMessenger.h"
#include "QueueFamily.h"
#include "Device.h"
#include "Queue.h"
#include "Swapchain.h"
#include "Renderpass.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MemoryLocation.h"
#include "Texture.h"
#include "Camera.h"
#include "ShaderTechnique.h"
#include "Skybox.h"

namespace vkn
{
	class Renderer
	{
	public:
		Renderer(gee::Window& window);
		~Renderer();
		std::ostream& getGpuInfo(std::ostream& os) const;
		void resize();
		void enableSkybox(bool value);
		void setSkybox(const std::array<std::string, 6>& skyboxPaths_);
		void setWindowMinimized(const bool value);
		void setRenderArea(const VkRect2D renderArea);
		void updateGui(std::function<void()> guiContent);
		void draw(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, std::vector<std::reference_wrapper<gee::Drawable>>& lights);
		void updateCamera(const gee::Camera& camera, const float aspectRatio);
		void setBackgroundColor(const glm::vec3& color);

	private:
		std::unique_ptr<vkn::Instance> instance_;
		std::unique_ptr<vkn::DebugMessenger> debugMessenger_;
		std::unique_ptr<vkn::Gpu> gpu_;
		VkSurfaceKHR surface_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::QueueFamily> queueFamily_;
		std::unique_ptr<vkn::Device> device_;
		std::unique_ptr<vkn::Swapchain> swapchain_;
		std::unique_ptr<vkn::Queue> graphicsQueue_;
		std::unique_ptr<vkn::Queue> transferQueue_;
		VkSampler sampler_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::CommandPool> cbPool_;
		std::vector<vkn::CommandBuffer> cbs_;
		std::vector<vkn::Signal> imageAvailableSignals_;
		std::vector<vkn::Signal> renderingFinishedSignals_;
		bool isWindowMinimized_{};
		std::unique_ptr<vkn::Skybox> skybox_;
		std::unique_ptr<vkn::ShaderTechnique> forwardRendering_;
		std::unique_ptr<vkn::ShaderTechnique> skyboxTechnique_;
		//imgui variables
		VkDescriptorPool imguiDescriptorPool_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::Renderpass> imguiRenderpass_;

		uint8_t currentFrame_{};
		uint8_t imageCount_{};
		VkViewport viewport_;
		VkRect2D renderArea_;
		glm::vec3 backgroundColor_{};

		void checkGpuCompability(const vkn::Gpu& gpu);
		void buildShaderTechnique();
		void createSampler();
		void buildImguiContext(const gee::Window& window);
		void record(const std::unordered_map<size_t, uint64_t>& sortedDrawables);
		void submit();
		std::function<void()> guiContent_;

		bool addMesh(const gee::Mesh& mesh);
		const size_t addMaterial(const gee::Material& material);
		const size_t addTexture(const gee::Texture& texture);
		void bindTexture(std::unordered_map<size_t, vkn::Image>& textures);
		void bindShaderMaterial(const std::unordered_map<size_t, gee::ShaderMaterial>& materials);
		void bindLights(std::vector<std::reference_wrapper<gee::Drawable>>& lights);
		vkn::Image createImageFromTexture(const gee::Texture& texture);
		const std::unordered_map<size_t, uint64_t> createSortedDrawables(std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		const glm::mat4 getModelMatrix(const gee::Drawable& drawable) const;
		std::vector<glm::mat4> modelMatrices_{};
		std::vector<glm::vec4> drawablesColors_{};
		std::unordered_map<size_t, vkn::MemoryLocation> meshesMemory_;
		std::unordered_map<size_t, gee::ShaderMaterial> shaderMaterials_;
		std::unordered_map<size_t, vkn::Image> textures_;
		bool skyboxEnbaled_{ true };
		struct MeshIndices
		{
			uint32_t modelIndices{};
			uint32_t materialIndex{};
		};
		std::vector<MeshIndices> meshesShaderIndices_;
		struct ShaderCamera
		{
			glm::vec4 position{};
			glm::mat4 view{};
			glm::mat4 projection{};
		};
	};
}