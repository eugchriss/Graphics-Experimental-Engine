#include "../../headers/impl/vulkan/Gpu.h"

using namespace gee;
vkn::Gpu::Gpu(const VkPhysicalDevice device_) : device{ device_ }
{
	descriptorIndexingFeatures_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	descriptorIndexingFeatures_.pNext = nullptr;
}

const VkPhysicalDeviceFeatures2& vkn::Gpu::enabledFeatures()
{
	VkPhysicalDeviceFeatures2 features{};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = &descriptorIndexingFeatures_;
	vkGetPhysicalDeviceFeatures2(device, &features);
	return features;
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
	return gpus;
}

const float vkn::Gpu::timeStamp() const
{
	auto props = properties();
	return props.limits.timestampPeriod;
}

const std::string vkn::Gpu::typeToString(const VkPhysicalDeviceType type) const
{
	switch (type)
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return "Discrete GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return "Integrated GPU";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return "Virtual GPU";
		break;
	default:
		return "Unknow GPU type";
		break;
	}
}

std::ostream& vkn::operator<<(std::ostream& os, const Gpu& gpu)
{
	auto props = gpu.properties();
	os << props.deviceName << " (" << gpu.typeToString(props.deviceType) << ")" << "\n";
	return os;
}
