#pragma once
#include "vulkanContext.h"
#include "Image.h"
#include "Texture.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include <memory>

namespace vkn
{
	class TextureImageFactory
	{
	public:
		TextureImageFactory(Context& _context);
		vkn::Image create(const gee::Texture& texture) const; 
	private:
		Context& context_;
		std::unique_ptr<vkn::CommandPool> cbPool_;
	};
}