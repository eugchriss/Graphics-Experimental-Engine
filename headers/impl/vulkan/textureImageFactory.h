#pragma once
#include <memory>

#include "../../Texture.h"
#include "CommandBuffer.h"
#include "Image.h"
#include "Queue.h"
#include "vulkanContext.h"

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