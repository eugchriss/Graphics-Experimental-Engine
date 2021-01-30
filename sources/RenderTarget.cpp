#include "..\headers\RenderTarget.h"

vkn::RenderTarget::RenderTarget(vkn::Context& _context, std::shared_ptr<Renderpass>& _renderpass, Framebuffer&& _framebuffer) :
	context_{ _context },
	renderpass { std::move(_renderpass) }, 
	framebuffer{ std::move(_framebuffer) },
	commandPool_{context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT }
{
	for (auto i = 0u; i < framebuffer.frameCount(); ++i)
	{
		cbs_.emplace_back(commandPool_.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		imageAvailableSignals_.emplace_back(context_, true);
		renderingFinishedSignals_.emplace_back(context_, true);
	}
}

void vkn::RenderTarget::clearDepthAttachment(const float clearColor)
{
	VkClearValue clear{};
	clear.depthStencil.depth = clearColor;
	renderpass->clearDepthAttachment(cbs_[currentFrame_], renderArea_, clear);
}

vkn::CommandBuffer& vkn::RenderTarget::bind(const VkRect2D& renderArea)
{
	renderArea_ = renderArea;
	auto& cb = cbs_[currentFrame_];
	cb.begin();
	renderpass->begin(cb, framebuffer.frame(currentFrame_), renderArea_, VK_SUBPASS_CONTENTS_INLINE);
	return cb;
}

vkn::Signal& vkn::RenderTarget::imageAvailableSignal()
{
	return imageAvailableSignals_[(currentFrame_ + 1) % framebuffer.frameCount()];
}

vkn::Signal& vkn::RenderTarget::renderingFinishedSignal()
{
	return renderingFinishedSignals_[(currentFrame_ + 1) % framebuffer.frameCount()];
}

bool vkn::RenderTarget::isReady()
{
	if (imageAvailableSignals_[currentFrame_].signaled() && renderingFinishedSignals_[currentFrame_].signaled())
	{
		renderingFinishedSignals_[currentFrame_].reset();
		return true;
	}
	return false;
}

void vkn::RenderTarget::unBind()
{
	renderpass->end(cbs_[currentFrame_]);
	cbs_[currentFrame_].end();
	++currentFrame_;
	currentFrame_ %= framebuffer.frameCount();
	assert(currentFrame_ < framebuffer.frameCount());
}

void vkn::RenderTarget::resize(const glm::u32vec2& size)
{
	currentFrame_ = 0;
	context_.device->idle();
	framebuffer.resize(size);
	cbs_.clear();
	imageAvailableSignals_.clear();
	renderingFinishedSignals_.clear();
	for (auto i = 0u; i < framebuffer.frameCount(); ++i)
	{
		cbs_.emplace_back(commandPool_.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY));
		imageAvailableSignals_.emplace_back(context_, true);
		renderingFinishedSignals_.emplace_back(context_, true);
	}
}
