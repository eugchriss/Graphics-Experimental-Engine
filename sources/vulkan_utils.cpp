#include <stdexcept>
#include "../headers/vulkan_utils.h"

void vkn::error_check(const VkResult result, const std::string& msg)
{
	if(result != VK_SUCCESS)
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
	else if(type.basetype == spirv_cross::SPIRType::UInt)
	{
		return { VK_FORMAT_R8G8B8A8_UINT, sizeof(uint32_t) };
	}
	else
	{
		throw std::runtime_error{ "vkn::getFormat received an untreated spirvType format \n" };
	}
}
