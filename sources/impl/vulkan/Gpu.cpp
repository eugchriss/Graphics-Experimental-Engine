#include "../../headers/impl/vulkan/Gpu.h"

using namespace gee;
vkn::Gpu::Gpu(const VkPhysicalDevice device_) : device{ device_ }
{
}

const VkPhysicalDeviceProperties vkn::Gpu::properties() const
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(device, &props);
	return props;
}

const VkPhysicalDeviceMemoryProperties vkn::Gpu::memoryProperties() const
{
	VkPhysicalDeviceMemoryProperties memProps{};
	vkGetPhysicalDeviceMemoryProperties(device, &memProps);
	return memProps;
}

const std::vector<vkn::Gpu> vkn::Gpu::getAvailbleGpus(const vkn::Instance& instance)
{
	uint32_t count{};
	vkEnumeratePhysicalDevices(instance.instance, &count, nullptr);
	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(instance.instance, &count, std::data(devices));
	
	std::vector<vkn::Gpu> gpus;
	for (auto device : devices)
	{
		gpus.emplace_back(device);
	}
	std::vector<VkPhysicalDeviceProperties> props;
	for (const auto gpu : gpus)
	{
		props.emplace_back(gpu.properties());
	}
	return gpus;
}

const float vkn::Gpu::timeStamp() const
{
	auto props = properties();
	return props.limits.timestampPeriod;
}

