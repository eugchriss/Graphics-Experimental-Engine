#pragma once
#include "vulkan/vulkan.hpp"
#include "Shader.h"
#include "Pipeline.h"
#include "vulkanContext.h"
#include "RenderTarget.h"
#include <vector>
#include <unordered_map>
#include <string>
namespace vkn
{
	class Renderpass;
	class Pass
	{
	public:
		struct RenderArea
		{
			VkViewport viewport;
			VkRect2D scissor;
		};

		Pass() = default;
		Pass(Pass&&) = default;
		~Pass() = default;
		void usesColorTarget(RenderTarget& target);
		void usesDepthStencilTarget(RenderTarget& target);
		void consumesTarget(RenderTarget& target);
		std::vector<RenderTargetRef>& colorTargets();
		std::vector<RenderTargetRef>& depthStencilTargets();
		std::vector<RenderTargetRef>& inputTargets();

	private:
		std::vector<RenderTargetRef> colorTargets_;
		std::vector<RenderTargetRef> depthStencilTargets_;
		std::vector<RenderTargetRef> inputTargets_;
	};
}