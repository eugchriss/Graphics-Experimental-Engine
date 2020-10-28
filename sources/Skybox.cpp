#include "..\headers\Skybox.h"
#include "../headers/Buffer.h"
#include "../headers/DeviceMemory.h"
#include "../headers/Texture.h"
#include "../headers/Signal.h"
#include "../headers/QueueFamily.h"
#include "../headers/Queue.h"
#include "../headers/CommandPool.h"
#include "../headers/CommandBuffer.h"
#include "glm/glm.hpp"
vkn::Skybox::Skybox(vkn::Gpu& gpu, vkn::Device& device, const std::array<std::string, 6>& paths)
{
	buildImage(gpu, device, paths);
	buildMesh(gpu, device);
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

void vkn::Skybox::buildImage(vkn::Gpu& gpu, vkn::Device& device, const std::array<std::string, 6>& paths)
{
	std::vector<gee::Texture> textures;
	for (const auto& path : paths)
	{
		textures.emplace_back(path);
	}

	std::vector<unsigned char> datas;
	std::vector<VkDeviceSize> texturesOffsets;
	size_t datasSize{};
	for (const auto& texture : textures)
	{
		texturesOffsets.push_back(datasSize);
		datasSize += std::size(texture.pixels());
	}
	datas.reserve(datasSize);
	for (const auto& texture : textures)
	{
		const auto& textureDatas = texture.pixels();
		std::copy(std::begin(textureDatas), std::end(textureDatas), std::back_inserter(datas));
	}

	vkn::Buffer buffer{ device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, datasSize };
	vkn::DeviceMemory memory{ gpu, device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer.getMemorySize() };
	buffer.bind(memory);
	buffer.add(datas);

	image_ = std::make_unique<vkn::Image>(gpu, device, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB, VkExtent3D{ textures[0].width(), textures[0].height(), 1 }, 6);

	vkn::QueueFamily transferQueueFamily{ gpu, VK_QUEUE_TRANSFER_BIT, 1 };
	vkn::CommandPool cbPool{ device, transferQueueFamily.familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	cb.begin();
	image_->copyFromBuffer(cb, VK_IMAGE_ASPECT_COLOR_BIT, buffer, texturesOffsets);
	image_->transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	cb.end();

	vkn::Signal imageReady{ device };
	auto transferQueue = transferQueueFamily.getQueue(device);
	transferQueue->submit(cb, imageReady);
	imageReady.waitForSignal();
}

void vkn::Skybox::buildMesh(vkn::Gpu& gpu, vkn::Device& device)
{
	std::vector<glm::vec3> vertices
	{
		glm::vec3{-1.0f, 1.0f, 1.0f},
		glm::vec3{1.0f, 1.0f, 1.0f},
		glm::vec3{1.0f, -1.0f, 1.0f},
		glm::vec3{-1.0f, -1.0f, 1.0f},
		glm::vec3{-1.0f, 1.0f, -1.0f},
		glm::vec3{1.0f, 1.0f, -1.0f},
		glm::vec3{1.0f, -1.0f, -1.0f},
		glm::vec3{-1.0f, -1.0f, -1.0f}
	};
	vertexBuffer_ = std::make_unique<vkn::Buffer>(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, std::size(vertices) * sizeof(glm::vec3));

	std::vector<uint32_t> indices
	{
		0, 1, 3, 1, 2, 3, //front
		4, 5, 7, 5, 6, 7, //back
		4, 0, 7, 0, 3, 7, //left
		1, 5, 2, 5, 6, 2, //right 
		4, 5, 0, 5, 1, 0, //up
		7, 6, 3, 6, 2, 3 // bottom
	};
	indexCount_ = std::size(indices);
	indexBuffer_ = std::make_unique<vkn::Buffer>(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, std::size(indices) * sizeof(uint32_t));

	hostVisibleMemory_ = std::make_unique<vkn::DeviceMemory>(gpu, device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer_->getMemorySize() + vertexBuffer_->getMemorySize());
	vertexBuffer_->bind(*hostVisibleMemory_);
	indexBuffer_->bind(*hostVisibleMemory_);

	vertexBuffer_->add(vertices);
	indexBuffer_->add(indices);
}
