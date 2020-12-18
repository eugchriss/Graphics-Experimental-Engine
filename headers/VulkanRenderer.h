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
#include "Swapchain.h"
#include "Renderpass.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "CommandPool.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Buffer.h"
#include "MeshMemoryLocation.h"
#include "Texture.h"
#include "Camera.h"
#include "AABB.h"
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
		void resize();
		const std::optional<size_t> objectAt(std::vector<std::reference_wrapper<gee::Drawable>>& drawables, const uint32_t x, const uint32_t y);
		void setWindowMinimized(const bool value);
		void updateGui(std::function<void()> guiContent);
		void render(vkn::Framebuffer& fb, vkn::ShaderEffect& effect, std::reference_wrapper<gee::Drawable>& drawable);
		void render(vkn::Framebuffer& fb, vkn::ShaderEffect& effect, const std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		void render(vkn::Framebuffer& fb, std::function<void()>& guiDatas);
		void draw(vkn::Framebuffer& fb);
		void updateCamera(const gee::Camera& camera, const float aspectRatio);
		std::unique_ptr<vkn::Framebuffer> getFramebuffer(std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount = 2u);
		vkn::Framebuffer createFramebuffer(const glm::u32vec2& extent, std::vector<vkn::ShaderEffect>& effects, const uint32_t frameCount = 2u);

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
		std::unique_ptr<vkn::Renderpass> imguiRenderpass_;

		void checkGpuCompability(const vkn::Gpu& gpu);
		void buildShaderTechnique();
		void createSampler();
		void buildImguiContext(const gee::Window& window);
		std::function<void()> guiContent_;

		void bindLights(std::vector<std::reference_wrapper<gee::Drawable>>& lights);
		void prepareBindingBox(std::vector<std::reference_wrapper<gee::Drawable>>& drawables);
		
		std::unique_ptr<vkn::MeshMemoryLocation> aabbMemoryLocation_;
		std::vector<glm::mat4> boundingBoxModels_;

		std::unique_ptr<MeshHolder_t> meshMemoryLocations_;
		std::unique_ptr<TextureHolder_t> textureHolder_;
		std::unique_ptr<MaterialHolder_t> materialHolder_;
	
		bool skyboxEnbaled_{ true };
		bool showBindingBox_{ false };
		
		ShaderCamera shaderCamera_{};
	};
}