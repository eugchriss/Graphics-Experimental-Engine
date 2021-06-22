#include <stdexcept>

#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;

std::unordered_map<gee::TargetFormat, VkFormat> vkn::Mapping::formats_ =
{
	{TargetFormat::D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}
};
std::unordered_map<gee::TargetUsage, VkImageUsageFlags> vkn::Mapping::usages_ =
{
		{COLOR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
		{DEPTH, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
		{SHADER_TEXTURE, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT },
		{READ, VK_IMAGE_USAGE_TRANSFER_SRC_BIT}
};
std::unordered_map<gee::TargetUsage, VkImageLayout> vkn::Mapping::layouts_ =
{
		{COLOR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
		{SHADER_TEXTURE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		{READ, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL }
};
std::unordered_map<gee::Filter, VkFilter> vkn::Mapping::filters_ =
{
	{NEAREST, VK_FILTER_NEAREST },
	{LINEAR, VK_FILTER_LINEAR}
};

std::unordered_map<gee::AddressMode, VkSamplerAddressMode> vkn::Mapping::addressMode_ =
{
	{REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT},
	{MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT},
	{CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
	{CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER}
};

std::unordered_map<gee::BorderColor, VkBorderColor> vkn::Mapping::borderColor_ =
{
	{TRANSPARENT_BLACK, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK},
	{OPAQUE_BLACK, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK},
	{OPAQUE_WHITE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE}
};

std::unordered_map<gee::CompareOp, VkCompareOp> vkn::Mapping::compareOp_ =
{
	{NEVER, VK_COMPARE_OP_NEVER},
	{LESS, VK_COMPARE_OP_LESS},
	{EQUAL, VK_COMPARE_OP_EQUAL},
	{LESS_OR_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL},
	{GREATER, VK_COMPARE_OP_GREATER},
	{NOT_EQUAL, VK_COMPARE_OP_NOT_EQUAL},
	{GREATER_OR_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL},
	{ALWAYS, VK_COMPARE_OP_ALWAYS}
};
std::unordered_map<gee::Filter, VkSamplerMipmapMode> vkn::Mapping::mipmapModes_
{
	{NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST },
	{LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR}
};

void vkn::error_check(const VkResult result, const std::string& msg)
{
	if (result != VK_SUCCESS)
		throw std::runtime_error{ msg };
}
const vkn::Format vkn::getFormat(const spirv_cross::SPIRType& type)
{
	if (type.basetype == spirv_cross::SPIRType::Float)
	{
		if (type.vecsize == 1)
		{
			return { VK_FORMAT_R32_SFLOAT, sizeof(float) };
		}
		else if (type.vecsize == 2)
		{
			return { VK_FORMAT_R32G32_SFLOAT, 2 * sizeof(float) };
		}
		else if (type.vecsize == 3)
		{
			return { VK_FORMAT_R32G32B32_SFLOAT, 3 * sizeof(float) };
		}
		else if (type.vecsize == 4)
		{
			return { VK_FORMAT_R32G32B32A32_SFLOAT, 4 * sizeof(float) };
		}
		else
		{
			throw std::runtime_error{ "vkn::getFormat received an untreated format \n" };
		}
	}
	else if (type.basetype == spirv_cross::SPIRType::UInt)
	{
		return { VK_FORMAT_R8G8B8A8_UINT, sizeof(uint32_t) };
	}
	else
	{
		throw std::runtime_error{ "vkn::getFormat received an untreated spirvType format \n" };
	}
}
