#pragma once
#include <vector>
#include <memory>
#include "vulkanContext.h"
#include "Renderpass.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Signal.h"

namespace vkn
{
	class RenderTarget
	{
	public:

		RenderTarget(Context& _context, std::shared_ptr<Renderpass>& _renderpass, Framebuffer&& _framebuffer);
		RenderTarget(RenderTarget&&) = default;
		void clearDepthAttachment(const float clearColor = 1.0f);
		CommandBuffer& bind(const VkRect2D& renderArea);
		Signal& imageAvailableSignal();
		Signal& renderingFinishedSignal();
		bool isReady();
		void unBind();
		void resize(const glm::u32vec2& size);
		float rawContextAt(const uint32_t x, const uint32_t y);
		const bool isOffscreen() const;
		std::shared_ptr<Renderpass> renderpass;
		Framebuffer framebuffer;
		friend class ImGuiContext;

	private:
		Context& context_;
		CommandPool commandPool_;
		std::vector<CommandBuffer> cbs_;
		uint32_t currentFrame_{};
		std::vector<Signal> imageAvailableSignals_;
		std::vector<Signal> renderingFinishedSignals_;
		VkRect2D renderArea_;
	};

}
