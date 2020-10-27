#pragma once
#include "vulkan/vulkan.hpp"
#include "Gpu.h"
#include "DeviceMemory.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include <unordered_map>
#include <memory>

namespace vkn
{
	class Image
	{
	public:
		//the default image layout is undefined
		Image(const vkn::Gpu& gpu, vkn::Device& device, const VkImageUsageFlags usage, const VkFormat format, const VkExtent3D extent, const uint32_t layerCount = 1);
		Image(vkn::Device& device, const VkImage image, const VkFormat format, const uint32_t layerCount = 1, bool owned = false);
		Image(Image&& image);
		~Image();

		void transitionLayout(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const VkImageLayout newLayout);
		void copyFromBuffer(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const vkn::Buffer& buffer, const VkDeviceSize offset);
		VkImage image{ VK_NULL_HANDLE };
		const VkImageView getView(const VkImageAspectFlags aspect, const VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t layerCount = 1);
		const VkImageView getView(const VkImageAspectFlags aspect, const VkImageViewType viewType, const VkFormat format, const uint32_t layerCount = 1);

	private:
		vkn::Device& device_;
		VkImageLayout layout_{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkFormat format_{};
		uint32_t layerCount_{};
		struct ViewType
		{
			VkImageViewType type{};
			VkImageAspectFlags aspect{};
			VkFormat format{};
			uint32_t layerCount{};
		};
		std::unordered_map<size_t, VkImageView> views_;
		std::unique_ptr<vkn::DeviceMemory> memory_;
		bool owned_{ true };
		VkExtent3D extent_{};
		VkImageView createView(const vkn::Image::ViewType& viewType);
	};
}