#pragma once
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <functional>
#include <utility>
#include <array>
#include <string>
#include <map>

#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"
#include "window.h"
#include "Instance.h"
#include "gpu.h"
#include "DebugMessenger.h"
#include "QueueFamily.h"
#include "Device.h"
#include "Queue.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "Drawable.h"
#include "Texture.h"
#include "Camera.h"
#include "ShaderEffect.h"
#include "ResourceHolder.h"

namespace vkn
{
	class Renderer
	{
	public:
		Renderer(gee::Window& window);
		~Renderer();
		std::ostream& getGpuInfo(std::ostream& os) const;
		const std::optional<size_t> objectAt(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, const uint32_t x, const uint32_t y);
		void render(const std::string& effectName, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		void render(vkn::Framebuffer& fb, const std::string& effectName, std::reference_wrapper<gee::Drawable>& drawable);
		void render(vkn::Framebuffer& fb, const std::string& effectName, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		void draw();
		void draw(vkn::Framebuffer& fb);
		void setViewport(const float x, const float y, const float width, const float height);
		void updateCamera(gee::Camera& camera, const float aspectRatio);
		vkn::Framebuffer& getFramebuffer();
		vkn::Framebuffer& getFramebuffer(std::vector<vkn::ShaderEffect>& effects, const bool enableGui = true);
		vkn::Framebuffer createFramebuffer(const glm::u32vec2& extent, std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount = 2u);
		void resize(const glm::u32vec2& size);
	private:
		std::unique_ptr<vkn::Instance> instance_;
		std::unique_ptr<vkn::DebugMessenger> debugMessenger_;
		std::unique_ptr<vkn::Gpu> gpu_;
		VkSurfaceKHR surface_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::QueueFamily> queueFamily_;
		std::unique_ptr<vkn::Device> device_;
		std::unique_ptr<vkn::Queue> graphicsQueue_;
		std::unique_ptr<vkn::Queue> transferQueue_;
		VkSampler sampler_{ VK_NULL_HANDLE };
		std::unique_ptr<vkn::CommandPool> cbPool_;
		bool isWindowMinimized_{};
		//imgui variables
		VkDescriptorPool imguiDescriptorPool_{ VK_NULL_HANDLE };
		ImGui_ImplVulkan_InitInfo guiInitInfo_{};
		void checkGpuCompability(const vkn::Gpu& gpu);
		void createSampler();
		void buildImguiContext(const gee::Window& window);
		
		std::vector<glm::mat4> boundingBoxModels_;

		std::unique_ptr<MeshHolder_t> meshMemoryLocations_;
		std::unique_ptr<TextureHolder_t> textureHolder_;
		std::unique_ptr<MaterialHolder_t> materialHolder_;
		std::unique_ptr<vkn::Framebuffer> mainFramebuffer_;
		std::unique_ptr<vkn::Framebuffer> pixelPerfectFramebuffer_;
		
		ShaderCamera shaderCamera_{};
	};
}