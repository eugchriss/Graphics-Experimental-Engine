#include "../headers/Image.h"
#include "../headers/vulkan_utils.h"

vkn::Image::Image(const vkn::Gpu& gpu, vkn::Device& device, VkImageUsageFlags usage, VkFormat format, VkExtent3D extent, const uint32_t layerCount) : device_{ device }, extent_{ extent }, format_{format}, layerCount_{layerCount}
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	if (layerCount_ == 6)
	{
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	else
	{
		imageInfo.flags = 0;
	}
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format_;
	imageInfo.extent = extent;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount_;
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
}

vkn::Image::Image(vkn::Device& device, const VkImage image, const VkFormat format, const uint32_t layerCount, bool owned) : device_{ device }, owned_{owned}, format_{format}, layerCount_{layerCount}
{
	this->image = image;
}

vkn::Image::Image(Image&& other): device_{other.device_}
{
	memory_ = std::move(other.memory_);
	views_ = std::move(other.views_);
	image = other.image;
	owned_ = other.owned_;
	format_ = other.format_;
	extent_ = other.extent_;
	layout_ = other.layout_;
	layerCount_ = other.layerCount_;

	other.image = VK_NULL_HANDLE;
}

vkn::Image::~Image()
{
	for(auto& [viewType, view] : views_)
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

VkImageView vkn::Image::createView(const vkn::Image::ViewType& viewType)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.flags = 0;
	viewInfo.image = image;
	viewInfo.viewType = viewType.type;
	viewInfo.format = viewType.format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	viewInfo.subresourceRange = { viewType.aspect, 0, 1, 0, viewType.layerCount };

	VkImageView view{};
	vkn::error_check(vkCreateImageView(device_.device, &viewInfo, nullptr, &view), "Failed to create the view");
	return view;
}

void vkn::Image::transitionLayout(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const VkImageLayout newLayout)
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
	imageBarrier.subresourceRange = { aspect, 0, 1, 0, layerCount_ };

	vkCmdPipelineBarrier(cb.commandBuffer(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
	
	layout_ = newLayout;
}

void vkn::Image::copyFromBuffer(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const vkn::Buffer& buffer, const VkDeviceSize offset)
{
	if (layout_ != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		transitionLayout(cb, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	VkBufferImageCopy copy{};
	copy.bufferOffset = offset;
	copy.imageExtent = extent_;
	copy.imageSubresource = { aspect, 0, 0, layerCount_ };

	vkCmdCopyBufferToImage(cb.commandBuffer(), buffer.buffer, image, layout_, 1, &copy);
}

void vkn::Image::copyFromBuffer(vkn::CommandBuffer& cb, const VkImageAspectFlags aspect, const vkn::Buffer& buffer, const std::vector<VkDeviceSize>& offsets)
{
	if (layout_ != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		transitionLayout(cb, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	std::vector<VkBufferImageCopy> copyRegions;	
	uint32_t layerIndex{};
	for (const auto offset : offsets)
	{
		VkBufferImageCopy copy{};
		copy.bufferOffset = offset;
		copy.imageExtent = extent_;
		copy.imageSubresource = { aspect, 0, layerIndex, 1 };
		copyRegions.push_back(copy);
		++layerIndex;
		assert(layerIndex <= layerCount_ && "Buffer copy layer is more than the image total layer count");
	}
	vkCmdCopyBufferToImage(cb.commandBuffer(), buffer.buffer, image, layout_, std::size(copyRegions), std::data(copyRegions));
}

const VkImageView vkn::Image::getView(const VkImageAspectFlags aspect, const VkImageViewType viewType, const uint32_t layerCount)
{
	assert(layerCount <= layerCount_ && "the view layer count must be less or equal to the image layer count");
	ViewType view;
	view.type = viewType;
	view.aspect = aspect;
	view.format = format_;
	view.layerCount = layerCount;
	size_t hash{ 0 };
	gee::hash_combine(hash, static_cast<size_t>(viewType), static_cast<size_t>(aspect), static_cast<size_t>(format_), layerCount);
	auto result = views_.find(hash);
	if (result != std::end(views_))
	{
		return result->second;
	}
	else
	{
		auto imageView = createView(view);
		views_[hash] = imageView;
		return imageView;
	}
}

const VkImageView vkn::Image::getView(const VkImageAspectFlags aspect, const VkImageViewType viewType, const VkFormat format, const uint32_t layerCount)
{
	ViewType view;
	view.type = viewType;
	view.aspect = aspect;
	view.format = format;
	view.layerCount = layerCount;
	size_t hash{ 0 };
	gee::hash_combine(hash, static_cast<size_t>(viewType), static_cast<size_t>(aspect), static_cast<size_t>(format_), layerCount);

	auto result = views_.find(hash);
	if (result != std::end(views_))
	{
		return result->second;
	}
	else
	{
		auto imageView = createView(view);
		views_[hash] = imageView;
		return imageView;
	}
}
