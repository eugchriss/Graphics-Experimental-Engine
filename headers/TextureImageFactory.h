#pragma once
#include <memory>
#include "Gpu.h"
#include "Device.h"
#include "Image.h"
#include "Texture.h"
#include "CommandPool.h"
#include "Queue.h"

namespace vkn
{
	class TextureImageFactory
	{
	public:
		TextureImageFactory(vkn::Gpu& gpu, vkn::Device& device);
		vkn::Image create(const gee::Texture& texture);
	private:
		vkn::Gpu& gpu_;
		vkn::Device& device_;
		std::unique_ptr<vkn::Queue> transferQueue_;
		std::unique_ptr<vkn::CommandPool> cbPool_;
	};
}