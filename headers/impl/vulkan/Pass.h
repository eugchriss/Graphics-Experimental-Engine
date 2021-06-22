#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Pipeline.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "vulkanContext.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
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
			void usesColorTarget(RenderTarget& target, const size_t index);
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
}