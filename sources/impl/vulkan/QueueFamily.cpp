#include <stdexcept>
#include <vector>

#include "../../headers/impl/vulkan/queue.h"
#include "../../headers/impl/vulkan/QueueFamily.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;
vkn::QueueFamily::QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, const uint32_t queueCount) : familyType_{ queueType }
{
	for (auto i = 0u; i < queueCount; ++i)
	{
		queuePriorities_.push_back(1 / static_cast<float>(queueCount));
	}

	uint32_t familyPropertyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> familyProperties(familyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, std::data(familyProperties));

	bool queueFamilyFound{ false };
	do
	{
		if ((familyProperties[familyIndex_].queueFlags & queueType) == queueType)
		{
			timestampValidBits_ = familyProperties[familyIndex_].timestampValidBits;
			queueFamilyFound = true;
		}
		else
		{
			++familyIndex_;
		}
		if (familyIndex_ > std::size(familyProperties))
		{
			throw std::runtime_error{ "unable to find a suitable queue family" };
		}
	} while (!queueFamilyFound && familyIndex_ <= std::size(familyProperties));

}
vkn::QueueFamily::QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, const std::vector<float>& queuePriorities) : queuePriorities_{ queuePriorities }, familyType_{ queueType }
{
	uint32_t familyPropertyCount{};
	std::vector<VkQueueFamilyProperties> familyProperties(familyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, std::data(familyProperties));

	bool queueFamilyFound{ false };
	do
	{
		if ((familyProperties[familyIndex_].queueFlags & queueType) == queueType)
		{
			timestampValidBits_ = familyProperties[familyIndex_].timestampValidBits;
			queueFamilyFound = true;
		}
		else
		{
			++familyIndex_;
		}
		if (familyIndex_ > std::size(familyProperties))
		{
			throw std::runtime_error{ "unable to find a suitable queue family" };
		}
	} while (!queueFamilyFound && familyIndex_ <= std::size(familyProperties));

}

vkn::QueueFamily::QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, VkSurfaceKHR surface, const uint32_t queueCount) : familyType_{ queueType }
{
	for (auto i = 0u; i < queueCount; ++i)
	{
		queuePriorities_.push_back(1 / static_cast<float>(queueCount));
	}

	uint32_t familyPropertyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> familyProperties(familyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, std::data(familyProperties));
	bool queueFamilyFound{ false };
	VkBool32 presentSupported{ VK_FALSE };
	do
	{
		if ((familyProperties[familyIndex_].queueFlags & queueType) == queueType)
		{
			timestampValidBits_ = familyProperties[familyIndex_].timestampValidBits;
			queueFamilyFound = true;
			vkn::error_check(vkGetPhysicalDeviceSurfaceSupportKHR(gpu.device, familyIndex_, surface, &presentSupported), "Unable to get surface support");
		}
		else
		{
			++familyIndex_;
		}
		if (familyIndex_ > std::size(familyProperties))
		{
			throw std::runtime_error{ "unable to find a suitable queue family" };
		}
	} while (!queueFamilyFound && !presentSupported && familyIndex_ <= std::size(familyProperties));
}

vkn::QueueFamily::QueueFamily(const vkn::Gpu& gpu, const VkQueueFlags queueType, VkSurfaceKHR surface, const std::vector<float>& queuePriorities) : queuePriorities_{ queuePriorities }, familyType_{ queueType }
{
	uint32_t familyPropertyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, nullptr);
	std::vector<VkQueueFamilyProperties> familyProperties(familyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu.device, &familyPropertyCount, std::data(familyProperties));

	bool queueFamilyFound{ false };
	VkBool32 presentSupported{ VK_FALSE };
	do
	{
		if ((familyProperties[familyIndex_].queueFlags & queueType) == queueType)
		{
			timestampValidBits_ = familyProperties[familyIndex_].timestampValidBits;
			queueFamilyFound = true;
			vkn::error_check(vkGetPhysicalDeviceSurfaceSupportKHR(gpu.device, familyIndex_, surface, &presentSupported), "Unable to get surface support");
		}
		else
		{
			++familyIndex_;
		}
		if (familyIndex_ > std::size(familyProperties))
		{
			throw std::runtime_error{ "unable to find a suitable queue family" };
		}
	} while (!queueFamilyFound && !presentSupported && familyIndex_ <= std::size(familyProperties));
}

std::unique_ptr<vkn::Queue> vkn::QueueFamily::getQueue(const vkn::Device& device)
{
	assert(queueIndex_ < std::size(queuePriorities_) && "The application requested more queues than reserved");
	auto queue = std::make_unique<vkn::Queue>(device, *this, queueIndex_);
	++queueIndex_;
	return queue;
}

uint32_t vkn::QueueFamily::timestampValidBits() const
{
	return timestampValidBits_;
}

const uint32_t vkn::QueueFamily::familyIndex() const
{
	return familyIndex_;
}

const VkDeviceQueueCreateInfo vkn::QueueFamily::info() const
{
	VkDeviceQueueCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.queueFamilyIndex = familyIndex_;
	info.queueCount = std::size(queuePriorities_); // one for graphics and one for transfer
	info.pQueuePriorities = std::data(queuePriorities_);
	return info;
}
