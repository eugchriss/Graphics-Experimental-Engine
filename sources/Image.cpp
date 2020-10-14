#include "../headers/Image.h"
#include "../headers/vulkan_utils.h"

vkn::Image::Image(const vkn::Gpu& gpu, vkn::Device& device, VkImageAspectFlags aspect, VkImageUsageFlags usage, VkFormat format, VkExtent3D extent) : device_{ device }, aspect_{ aspect }, extent_{ extent }
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = extent;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkn::error_check(vkCreateImage(device_.device, &imageInfo, nullptr, &image), "Failed to create the image");

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(device_.device, image, &memRequirements);
	memory_ = std::make_unique<vkn::DeviceMemory>(gpu.device, device_, memRequirements.memoryTypeBits, memRequirements.size);
	memory_->bind(image);

	createView(format, aspect_);
}

vkn::Image::Image(vkn::Device& device, const VkImage image, const VkFormat format, const VkImageAspectFlags aspect, bool owned) : device_{ device }, owned_{owned},	aspect_{aspect}
{
	this->image = image;
	createView(format, aspect_);
}

vkn::Image::Image(Image&& other): device_{other.device_}
{
	memory_ = std::move(other.memory_);
	image = other.image;
	view = other.view;
	owned_ = other.owned_;
	aspect_ = other.aspect_;
	extent_ = other.extent_;
	layout_ = other.layout_;

	other.image = VK_NULL_HANDLE;
	other.view = VK_NULL_HANDLE;
}

vkn::Image::~Image()
{
	if (view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device_.device, view, nullptr);
	}
	if (owned_)
	{
		if (image != VK_NULL_HANDLE)
		{
			vkDestroyImage(device_.device, image, nullptr);
		}
	}
}

void vkn::Image::createView(const VkFormat format, const VkImageAspectFlags aspect)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.flags = 0;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	viewInfo.subresourceRange = { aspect, 0, 1, 0, 1 };
	vkn::error_check(vkCreateImageView(device_.device, &viewInfo, nullptr, &view), "Failed to create the view");
}

void vkn::Image::transitionLayout(vkn::CommandBuffer& cb, const VkImageLayout newLayout)
{
	VkPipelineStageFlagBits srcStage{};
	VkPipelineStageFlagBits dstStage{};
	VkImageMemoryBarrier imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = nullptr;
	imageBarrier.oldLayout = layout_;
	imageBarrier.newLayout = newLayout;

	if (layout_ == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		imageBarrier.srcAccessMask = 0;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}
	else if (layout_ == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (layout_ == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (layout_ == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	else if (layout_ == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if ((newLayout & VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if ((newLayout & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	imageBarrier.image = image;
	imageBarrier.subresourceRange = { aspect_, 0, 1, 0, 1 };

	vkCmdPipelineBarrier(cb.commandBuffer(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
	
	layout_ = newLayout;
}

void vkn::Image::copyFromBuffer(vkn::CommandBuffer& cb, const vkn::Buffer& buffer, const VkDeviceSize offset)
{
	if (layout_ != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		transitionLayout(cb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	VkBufferImageCopy copy{};
	copy.bufferOffset = offset;
	copy.imageExtent = extent_;
	copy.imageSubresource = { aspect_, 0, 0, 1 };

	vkCmdCopyBufferToImage(cb.commandBuffer(), buffer.buffer, image, layout_, 1, &copy);
}
