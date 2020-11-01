#include "..\headers\Skybox.h"
#include "../headers/Buffer.h"
#include "../headers/DeviceMemory.h"
#include "../headers/Texture.h"
#include "../headers/Signal.h"
#include "../headers/QueueFamily.h"
#include "../headers/Queue.h"
#include "../headers/CommandPool.h"
#include "../headers/CommandBuffer.h"
#include "../headers/utility.h"

#include <future>

vkn::Skybox::Skybox(vkn::Gpu& gpu, vkn::Device& device, const std::array<std::string, 6>& paths)
{
	vkn::QueueFamily transferQueueFamily{ gpu, VK_QUEUE_TRANSFER_BIT, 1 };
	auto transferQueue = transferQueueFamily.getQueue(device);

	buildImage(gpu, device, *transferQueue, paths);
	buildMesh(gpu, device, *transferQueue);
}

VkImageView vkn::Skybox::getView()
{
	return image_->getView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);
}

const VkBuffer& vkn::Skybox::vertexBuffer() const
{
	return vertexBuffer_->buffer;
}

const VkBuffer& vkn::Skybox::indexBuffer() const
{
	return indexBuffer_->buffer;
}

const uint32_t vkn::Skybox::indexCount() const
{
	return indexCount_;
}

void vkn::Skybox::buildImage(vkn::Gpu& gpu, vkn::Device& device, vkn::Queue& queue, const std::array<std::string, 6>& paths)
{
	std::vector<VkDeviceSize> texturesOffsets;
	size_t datasSize{};
	std::vector<gee::Texture> textures;
	for (const auto& path : paths)
	{
		textures.emplace_back(path);
		texturesOffsets.push_back(datasSize);
		datasSize += std::size(textures.back().pixels());
	}

	std::vector<unsigned char> datas(datasSize);
	uint32_t offset{};
	for (const auto& texture : textures)
	{
		const auto& textureDatas = texture.pixels();
		std::async(std::launch::async, [&]()
		{
			std::copy_n(std::begin(textureDatas), std::size(textureDatas), std::begin(datas) + offset);
		});
		offset += std::size(textureDatas);
	}

	vkn::Buffer buffer{ device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, datasSize };
	vkn::DeviceMemory memory{ gpu, device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer.getMemorySize() };
	buffer.bind(memory);
	buffer.add(datas);

	image_ = std::make_unique<vkn::Image>(gpu, device, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB, VkExtent3D{ textures[0].width(), textures[0].height(), 1 }, 6);

	vkn::CommandPool cbPool{ device, queue.familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	image_->copyFromBuffer(cb, VK_IMAGE_ASPECT_COLOR_BIT, buffer, texturesOffsets);
	image_->transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cb.end();

	vkn::Signal imageReady{ device };
	queue.submit(cb, imageReady);
	imageReady.waitForSignal();
}

void vkn::Skybox::buildMesh(vkn::Gpu& gpu, vkn::Device& device, vkn::Queue& queue)
{
	gee::Cube cube;
	vertexBuffer_ = std::make_unique<vkn::Buffer>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, std::size(cube.vertices) * sizeof(gee::Vertex));

	indexCount_ = std::size(cube.indices);
	indexBuffer_ = std::make_unique<vkn::Buffer>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, std::size(cube.indices) * sizeof(uint32_t));

	vkn::DeviceMemory temp{ gpu, device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer_->getMemorySize() + vertexBuffer_->getMemorySize() };
	vertexBuffer_->bind(temp);
	indexBuffer_->bind(temp);

	vertexBuffer_->add(cube.vertices);
	indexBuffer_->add(cube.indices);
	
	hostVisibleMemory_ = std::make_unique<vkn::DeviceMemory>(gpu, device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer_->getMemorySize() + vertexBuffer_->getMemorySize());
	vertexBuffer_->moveTo(queue, *hostVisibleMemory_);
	indexBuffer_->moveTo(queue, *hostVisibleMemory_);
}
