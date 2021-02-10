#include "..\headers\RenderTarget.h"

vkn::RenderTarget::RenderTarget(vkn::Context& _context, std::shared_ptr<Renderpass>& _renderpass, Framebuffer&& _framebuffer) :
	context_{ _context },
	renderpass { std::move(_renderpass) }, 
	framebuffer{ std::move(_framebuffer) }
{
	for (auto i = 0u; i < framebuffer.frameCount(); ++i)
	{
		imageAvailableSignals_.emplace_back(context_);
	}
}

void vkn::RenderTarget::clearDepthAttachment(CommandBuffer& cb, const float clearColor)
{
	VkClearValue clear{};
	clear.depthStencil.depth = clearColor;
	renderpass->clearDepthAttachment(cb, renderArea_, clear);
}

void  vkn::RenderTarget::bind(CommandBuffer& cb, const VkRect2D& renderArea)
{
	renderArea_ = renderArea;
	renderpass->begin(cb, framebuffer.frame(currentFrame_), renderArea_, VK_SUBPASS_CONTENTS_INLINE);
	isBound_ = true;
}

bool vkn::RenderTarget::isReady(Swapchain& swapchain)
{
	swapchain.setImageAvailableSignal(imageAvailableSignals_[currentFrame_]);
	return imageAvailableSignals_[currentFrame_].signaled();
}

void vkn::RenderTarget::unBind(CommandBuffer& cb)
{
	renderpass->end(cb);
	++currentFrame_;
	currentFrame_ %= framebuffer.frameCount();
	assert(currentFrame_ < framebuffer.frameCount());
	isBound_ = false;
}

void vkn::RenderTarget::resize(const glm::u32vec2& size)
{
	currentFrame_ = 0;
	context_.device->idle();
	framebuffer.resize(size);
	imageAvailableSignals_.clear();
	for (auto i = 0u; i < framebuffer.frameCount(); ++i)
	{
		imageAvailableSignals_.emplace_back(context_);
	}
}

float vkn::RenderTarget::rawContextAt(const uint32_t x, const uint32_t y)
{	
	auto offset = (x + y * framebuffer.dimensions().width) * 4;
	return framebuffer.rawContentAt(offset);
}

const bool vkn::RenderTarget::isOffscreen() const
{
	return framebuffer.isOffscreen();
}

const bool vkn::RenderTarget::isBound() const
{
	return isBound_;
}
