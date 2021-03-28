#pragma once
#include "vulkan/vulkan.hpp"
#include "CommandBuffer.h"
#include "vulkanContext.h"
#include "Buffer.h"
#include "DeviceMemory.h"
#include <unordered_map>
#include <memory>

namespace vkn
{
	struct Pixel
	{
		float r;
		float g;
		float b;
		float a;
	};
	class Image
	{
	public:
		//the default image layout is undefined
		Image(Context& context, const VkImageUsageFlags usage, const VkFormat format, const VkExtent3D extent, const uint32_t layerCount = 1);
		Image(Context& context, const VkImage image, const VkFormat format, const uint32_t layerCount = 1, bool owned = false);
		Image(Image&& image);
		~Image();

#ifndef NDEBUG
		void setDebugName(const std::string& name);
#endif
		const std::vector<Pixel> content(const vkn::Gpu& gpu, const VkImageAspectFlags& apect = VK_IMAGE_ASPECT_COLOR_BIT);
		const float rawContentAt(const vkn::Gpu& gpu, const VkDeviceSize offset, const VkImageAspectFlags& apect = VK_IMAGE_ASPECT_COLOR_BIT);
		const std::vector<float> rawContent(const vkn::Gpu& gpu, const VkImageAspectFlags& apect = VK_IMAGE_ASPECT_COLOR_BIT);
		void copyToBuffer(vkn::CommandBuffer& cb, vkn::Buffer& buffer, const VkImageAspectFlags& aspect);
		void copyToImage(vkn::CommandBuffer& cb, vkn::Image& image, const VkImageAspectFlags aspect);
		void transitionLayout(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const VkImageLayout newLayout);
		void copyFromBuffer(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const vkn::Buffer& buffer, const VkDeviceSize offset);
		void copyFromBuffer(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const vkn::Buffer& buffer, const std::vector<VkDeviceSize>& offsets);
		VkImage image{ VK_NULL_HANDLE };
		const VkImageView getView(const VkImageAspectFlags aspect, const VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, const uint32_t layerCount = 1);
		const VkImageView getView(const VkImageAspectFlags aspect, const VkImageViewType viewType, const VkFormat format, const uint32_t layerCount = 1);
	private:
		std::string name_;//only used for debugging purpose
		Context& context_;
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
		VkDeviceSize memoryOffset_{};
		bool owned_{ true };
		VkExtent3D extent_{};
		VkImageUsageFlags getUsageFlag(const VkAttachmentDescription& attachment);
		VkImageView createView(const vkn::Image::ViewType& viewType);
		const VkMemoryRequirements getMemoryRequirement() const;
		const std::string getStringUsage(const VkImageUsageFlags usageFlag)const;
		const std::vector<uint32_t> unwrapFlags(const VkImageUsageFlags usageFlag) const;
		const std::string getStringViewType(const VkImageViewType type)const;
	};
}