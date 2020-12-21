#include "..\headers\textureImageFactory.h"
#include "../headers/CommandPool.h"
#include "../headers/Signal.h"
#include "../headers/QueueFamily.h"

vkn::TextureImageFactory::TextureImageFactory(vkn::Gpu& gpu, vkn::Device& device): gpu_{gpu}, device_{device}
{
	vkn::QueueFamily queueFamily{ gpu_, VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_TRANSFER_BIT, 2 };
	cbPool_ = std::make_unique<vkn::CommandPool>(device_, queueFamily.familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
	transferQueue_ = queueFamily.getQueue(device_);
}

vkn::Image vkn::TextureImageFactory::create(const gee::Texture& texture) const
{
	const auto& datas = texture.pixels();
	const auto textureSize = std::size(datas);
	vkn::Buffer temp{ device_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, textureSize };
	vkn::DeviceMemory memory{ gpu_, device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, temp.getMemorySize() };
	temp.bind(memory);
	temp.add(datas);

	VkFormat imageFormat{ VK_FORMAT_R8G8B8A8_SRGB };
	if (texture.colorSpace() == gee::Texture::ColorSpace::LINEAR)
	{
		imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	}

	const uint32_t layerCount = std::size(texture.offsets());
	vkn::Image image{ gpu_, device_, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageFormat, VkExtent3D{texture.width(), texture.height(), 1}, layerCount };

	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	image.copyFromBuffer(cb, VK_IMAGE_ASPECT_COLOR_BIT, temp, 0);
	image.transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cb.end();

	vkn::Signal imageReady{ device_ };
	transferQueue_->submit(cb, imageReady);
	imageReady.waitForSignal();
	return std::move(image);
}
