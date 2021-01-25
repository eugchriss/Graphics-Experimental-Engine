#include "..\headers\textureImageFactory.h"
#include "../headers/CommandPool.h"
#include "../headers/Signal.h"
#include "../headers/QueueFamily.h"

vkn::TextureImageFactory::TextureImageFactory(Context& context): context_{context}
{
	cbPool_ = std::make_unique<vkn::CommandPool>(context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
}

vkn::Image vkn::TextureImageFactory::create(const gee::Texture& texture) const
{
	const auto& datas = texture.pixels();
	const auto textureSize = std::size(datas);
	vkn::Buffer temp{ context_, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, textureSize };
	vkn::DeviceMemory memory{ *context_.gpu, *context_.device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, temp.getMemorySize() };
	temp.bind(memory);
	temp.add(datas);

	VkFormat imageFormat{ VK_FORMAT_R8G8B8A8_SRGB };
	if (texture.colorSpace() == gee::Texture::ColorSpace::LINEAR)
	{
		imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	}

	const uint32_t layerCount = std::size(texture.offsets());
	vkn::Image image{ context_, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageFormat, VkExtent3D{texture.width(), texture.height(), 1}, layerCount };

	auto cb = cbPool_->getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	image.copyFromBuffer(cb, VK_IMAGE_ASPECT_COLOR_BIT, temp, 0);
	image.transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cb.end();

	vkn::Signal imageReady{ context_ };
	context_.transferQueue->submit(cb, imageReady);
	imageReady.waitForSignal();
	return std::move(image);
}
