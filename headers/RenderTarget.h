#pragma once
#include <vector>
#include <memory>
#include <string>
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
		float rawContextAt(const std::string& attachmentName, const uint32_t x, const uint32_t y);
		const bool isOffscreen() const;
		const bool isBound() const;
		vkn::Image& attachmentImage(const std::string& name);
		std::shared_ptr<Renderpass> renderpass;
		Framebuffer framebuffer;

	private:
		Context& context_;
		uint32_t currentFrame_{};
		std::vector<Signal> imageAvailableSignals_;
		VkRect2D renderArea_;
		bool isBound_{ false };
	};

}
