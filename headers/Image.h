#pragma once
#include "vulkan/vulkan.hpp"
#include "Gpu.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "CommandBuffer.h"

#include <memory>

namespace vkn
{
	class Image
	{
	public:
		//the default image layout is undefined
		Image(const vkn::Gpu& gpu, vkn::Device& device, const VkImageAspectFlags apsect, const VkImageUsageFlags usage, const VkFormat format, const VkExtent3D extent);
		Image(vkn::Device& device, const VkImage image, const VkFormat format, const VkImageAspectFlags apsect, bool owned = false);
		Image(Image&& image);
		~Image();

		void transitionLayout(vkn::CommandBuffer& cb, const VkImageLayout newLayout);
		void copyFromBuffer(vkn::CommandBuffer& cb, const vkn::Buffer& buffer, const VkDeviceSize offset);
		VkImage image{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };

	private:
		vkn::Device& device_;
		VkImageLayout layout_{ VK_IMAGE_LAYOUT_UNDEFINED};
		std::unique_ptr<vkn::DeviceMemory> memory_;
		bool owned_{ true };
		VkImageAspectFlags aspect_{};
		VkExtent3D extent_{};
		void createView(const VkFormat format, const VkImageAspectFlags aspect);
	};
}