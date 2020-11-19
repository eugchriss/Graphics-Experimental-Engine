#include "../headers/Image.h"
#include "../headers/vulkan_utils.h"
#include "../headers/QueueFamily.h"
#include "../headers/Queue.h"
#include "../headers/CommandPool.h"
#include "../headers/CommandBuffer.h"
#include "../headers/Signal.h"

vkn::Image::Image(const vkn::Gpu& gpu, vkn::Device& device, VkImageUsageFlags usage, VkFormat format, VkExtent3D extent, const uint32_t layerCount) : device_{ device }, extent_{ extent }, format_{ format }, layerCount_{ layerCount }
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
	imageInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkn::error_check(vkCreateImage(device_.device, &imageInfo, nullptr, &image), "Failed to create the image");

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(device_.device, image, &memRequirements);
	memory_ = std::make_unique<vkn::DeviceMemory>(gpu.device, device_, memRequirements.memoryTypeBits, memRequirements.size);
	memory_->bind(image);

#ifndef NDEBUG
	name_ = "Owned image";
	setDebugName(name_ + getStringUsage(usage));
#endif
}

vkn::Image::Image(vkn::Device& device, const VkImage image, const VkFormat format, const uint32_t layerCount, bool owned) : device_{ device }, owned_{ owned }, format_{ format }, layerCount_{ layerCount }
{
	this->image = image;
#ifndef NDEBUG
	name_ = "Non owned image";
	setDebugName(name_);
#endif
}

vkn::Image::Image(Image&& other) : device_{ other.device_ }
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
	for (auto& [viewType, view] : views_)
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

#ifndef NDEBUG
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.pNext = nullptr;
	nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(image);
	nameInfo.pObjectName = std::string{ name_ + getStringViewType(viewType.type) }.c_str();
	device_.setDebugOjectName(nameInfo);
	return view;
#endif
}

const VkMemoryRequirements vkn::Image::getMemoryRequirement() const
{
	VkMemoryRequirements requirements{};
	vkGetImageMemoryRequirements(device_.device, image, &requirements);
	return requirements;
}
#ifndef NDEBUG
void vkn::Image::setDebugName(const std::string& name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.pNext = nullptr;
	nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(image);
	nameInfo.pObjectName = name.c_str();

	device_.setDebugOjectName(nameInfo);
}
#endif
const std::vector<vkn::Pixel> vkn::Image::content(const vkn::Gpu& gpu, const VkImageAspectFlags& aspect)
{
	auto& raw = rawContent(gpu, aspect);
	assert(std::size(raw) % 4 == 0 && "image raw content is corrupted");
	std::vector<vkn::Pixel> datas;
	datas.reserve(std::size(raw)/4);
	for (auto i = 0u; i < std::size(raw); i += 4)
	{
		Pixel p;
		p.r = raw[i];
		p.g = raw[i + 1];
		p.b = raw[i + 2];
		p.a = raw[i + 3];
		datas.push_back(p);
	}
	return std::move(datas);
}

const std::vector<float> vkn::Image::rawContent(const vkn::Gpu& gpu, const VkImageAspectFlags& aspect)
{
	auto requirements = getMemoryRequirement();
	vkn::Buffer buffer{ device_, VK_BUFFER_USAGE_TRANSFER_DST_BIT, requirements.size };
	vkn::DeviceMemory memory{ gpu, device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, buffer.getMemorySize() };
	buffer.bind(memory);

	vkn::QueueFamily transferQueueFamily{ gpu, VK_QUEUE_TRANSFER_BIT, 1 };
	vkn:CommandPool cbPool{ device_, transferQueueFamily.familyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	auto cb = cbPool.getCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	cb.begin();
	copyToBuffer(cb, buffer, VK_IMAGE_ASPECT_COLOR_BIT);
	cb.end();

	auto transferQueue = transferQueueFamily.getQueue(device_);
	vkn::Signal copyDone{ device_ };
	transferQueue->submit(cb, copyDone);
	copyDone.waitForSignal();

	return buffer.rawContent();
}

void vkn::Image::copyToBuffer(vkn::CommandBuffer& cb, vkn::Buffer& buffer, const VkImageAspectFlags& aspect)
{
	VkBufferImageCopy copy{};
	copy.bufferImageHeight = 0;
	copy.bufferRowLength = 0;
	copy.bufferOffset = 0;
	copy.imageOffset = VkOffset3D{};
	copy.imageExtent = extent_;
	copy.imageSubresource = { aspect, 0, 0, layerCount_ };

	auto layout = layout_;
	transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	vkCmdCopyImageToBuffer(cb.commandBuffer(), image, layout_, buffer.buffer, 1, &copy);
	transitionLayout(cb, VK_IMAGE_ASPECT_COLOR_BIT, layout);
}

void vkn::Image::copyToImage(vkn::CommandBuffer& cb, vkn::Image& dstImage, const VkImageAspectFlags aspect)
{
	auto srcLayout = layout_;
	auto dstLayout = dstImage.layout_;

	transitionLayout(cb, aspect, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	dstImage.transitionLayout(cb, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VkImageCopy copy{};
	copy.srcSubresource = { aspect, 0, 0, layerCount_ };
	copy.srcOffset = VkOffset3D{ 0, 0, 0 };
	copy.dstSubresource = { aspect, 0, 0, dstImage.layerCount_ };
	copy.dstOffset = VkOffset3D{ 0, 0, 0 };
	copy.extent = extent_;
	vkCmdCopyImage(cb.commandBuffer(), image, layout_, dstImage.image, dstImage.layout_, 1, &copy);
	transitionLayout(cb, aspect, srcLayout);
	dstImage.transitionLayout(cb, aspect, dstLayout);
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
	else if (layout_ == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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
	else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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

const std::string vkn::Image::getStringUsage(const VkImageUsageFlags usageFlag) const
{
	auto bits = unwrapFlags(usageFlag);
	std::string usage{};
	for (const auto flag : bits)
	{
		switch (1 << flag)
		{
		case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
			usage += std::string{" USAGE_TRANSFER_SRC" };
			break;
		case VK_IMAGE_USAGE_TRANSFER_DST_BIT:
			usage += std::string{" USAGE_TRANSFER_DST" };
			break;
		case VK_IMAGE_USAGE_SAMPLED_BIT:
			usage += std::string{" USAGE_SAMPLED" };
			break;
		case VK_IMAGE_USAGE_STORAGE_BIT:
			usage += std::string{" USAGE_STORAGE" };
			break;
		case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
			usage += std::string{" USAGE_COLOR_ATTACHMENT" };
			break;
		case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:
			usage += std::string{" USAGE_DEPTH_STENCIL_ATTACHMENT" };
			break;
		case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT:
			usage += std::string{" USAGE_TRANSIENT_ATTACHMENT" };
			break;
		case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT:
			usage += std::string{" USAGE_INPUT_ATTACHMENT" };
			break;
		default:
			usage = std::string{ "corrupted image usage" };
			break;
		}
	}
	return std::string();
}

const std::vector<uint32_t> vkn::Image::unwrapFlags(const VkImageUsageFlags usageFlag) const
{
	std::vector<uint32_t> bits;
	for (auto i = 0u; i < 8; ++i)
	{
		if (usageFlag & (1 << i) == i)
		{
			bits.push_back(i);
		}
	}
	return bits;
}

const std::string vkn::Image::getStringViewType(const VkImageViewType type) const
{
	std::string str;
	switch (type)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
		str = "VIEW_TYPE_1D";
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
		str = "VIEW_TYPE_2D";
		break;
	case VK_IMAGE_VIEW_TYPE_3D:
		str = "VIEW_TYPE_3D";
	break;
		case VK_IMAGE_VIEW_TYPE_CUBE:
		str = "VIEW_TYPE_CUBE";
		break;
	default:
		str = "unknown view type";
		break;
	}
	return str;
}

