#pragma once
#include <vector>
#include <memory>
#include "vulkanContext.h"
#include "Renderpass.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "Signal.h"
#include "Swapchain.h"

namespace vkn
{
	class RenderTarget
	{
	public:

		RenderTarget(Context& _context, std::shared_ptr<Renderpass>& _renderpass, Framebuffer&& _framebuffer);
		RenderTarget(RenderTarget&&) = default;
		void clearDepthAttachment(CommandBuffer& cb, const float clearColor = 1.0f);
		void bind(CommandBuffer& cb, const VkRect2D& renderArea);
		bool isReady(Swapchain& swapchain);
		void unBind(CommandBuffer& cb);
		void resize(const glm::u32vec2& size);
		float rawContextAt(const uint32_t x, const uint32_t y);
		const bool isOffscreen() const;
		std::shared_ptr<Renderpass> renderpass;
		Framebuffer framebuffer;
		friend class ImGuiContext;

	private:
		Context& context_;
		uint32_t currentFrame_{};
		std::vector<Signal> imageAvailableSignals_;
		VkRect2D renderArea_;
	};

}
