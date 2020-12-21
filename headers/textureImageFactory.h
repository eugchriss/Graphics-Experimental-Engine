#pragma once
#include "Gpu.h"
#include "Device.h"
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
		TextureImageFactory(vkn::Gpu& gpu, vkn::Device& device);
		vkn::Image create(const gee::Texture& texture) const; 
	private:
		vkn::Gpu& gpu_;
		vkn::Device& device_;
		std::unique_ptr<vkn::CommandPool> cbPool_;
		std::unique_ptr<vkn::Queue> transferQueue_;
	};
}