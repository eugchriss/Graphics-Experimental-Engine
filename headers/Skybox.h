#pragma once
#include "Device.h"
#include "Gpu.h"
#include "Image.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include <string>
#include <array>
#include <memory>
#include <vector>

namespace vkn
{
	class Skybox
	{
	public:
		Skybox(vkn::Gpu& gpu, vkn::Device& device, const std::array<std::string, 6>& paths);
		Skybox(Skybox&&) = default;
		VkImageView getView();
		const VkBuffer& vertexBuffer() const;
		const VkBuffer& indexBuffer() const;
		const uint32_t indexCount() const;
	private:
		std::unique_ptr<vkn::Image> image_;
		std::unique_ptr<vkn::Buffer> vertexBuffer_;
		std::unique_ptr<vkn::Buffer> indexBuffer_;
		std::unique_ptr<vkn::DeviceMemory> hostVisibleMemory_;
		uint32_t indexCount_;
		void buildImage(vkn::Gpu& gpu, vkn::Device& device, vkn::Queue& queue, const std::array<std::string, 6>& paths);
		void buildMesh(vkn::Gpu& gpu, vkn::Device& device, vkn::Queue& queue);
	};

}
