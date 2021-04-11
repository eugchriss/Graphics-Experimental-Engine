#include "../headers/PipelineLayout.h"
#include "../headers/vulkan_utils.h"

#include <unordered_map>

vkn::PipelineLayout::PipelineLayout(vkn::Device& device, const std::vector<vkn::Shader>& shaders) : device_{ device }
{
	auto setLayouts = createSets(shaders);
	createPushConstantRanges(shaders);
	create_descriptor_pool(shaders);

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0;
	layoutInfo.pushConstantRangeCount = std::size(pushConstantRanges_);
	layoutInfo.pPushConstantRanges = std::data(pushConstantRanges_);
	layoutInfo.setLayoutCount = std::size(setLayouts);
	layoutInfo.pSetLayouts = std::data(setLayouts);
	vkn::error_check(vkCreatePipelineLayout(device.device, &layoutInfo, nullptr, &layout), "Failed to create the pipeline Layout");

	if (!std::empty(setLayouts))
	{
		for (auto layout : setLayouts)
		{
			vkDestroyDescriptorSetLayout(device_.device, layout, nullptr);
		}
	}
}

vkn::PipelineLayout::PipelineLayout(PipelineLayout&& other) : device_{ other.device_ }
{
	layout = other.layout;
	sets_ = std::move(other.sets_);
	descriptorPool_ = other.descriptorPool_;
	other.layout = VK_NULL_HANDLE;
	other.descriptorPool_ = VK_NULL_HANDLE;
}

vkn::PipelineLayout::~PipelineLayout()
{
	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device_.device, layout, nullptr);
	}
	if (!std::empty(sets_))
	{
		vkFreeDescriptorSets(device_.device, descriptorPool_, std::size(sets_), std::data(sets_));
	}
	if (descriptorPool_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device_.device, descriptorPool_, nullptr);
	}
}

const std::vector<VkDescriptorSet>& vkn::PipelineLayout::sets() const
{
	return sets_;
}

void vkn::PipelineLayout::create_descriptor_pool(const std::vector<Shader>& shaders)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& shader : shaders)
	{
		auto& poolSize = shader.poolSize();
		std::copy(std::begin(poolSize), std::end(poolSize), std::back_inserter(poolSizes));
	}
	VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 5;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes = std::data(poolSizes);
	vkCreateDescriptorPool(device_.device, &poolInfo, nullptr, &descriptorPool_);
}

const std::vector<VkDescriptorSetLayout> vkn::PipelineLayout::createSets(const std::vector<vkn::Shader>& shaders)
{
	std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setBindings;
	for (const auto& shader : shaders)
	{
		auto& shaderBindings = shader.bindings();
		for (const auto& shaderBinding : shaderBindings)
		{
			setBindings[shaderBinding.set].push_back(shaderBinding.layoutBinding);
		}
		auto& subpassInputBindings = shader.subpassInputBindings();
		for (const auto& subpassInputBinding : subpassInputBindings)
		{
			setBindings[subpassInputBinding.set].push_back(subpassInputBinding.layoutBinding);
		}
	}

	std::vector<VkDescriptorSetLayout> layouts;
	for (const auto& setBinding : setBindings)
	{
		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };

		VkDescriptorSetLayoutCreateInfo setInfo{};
		setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setInfo.pNext = nullptr;
		setInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
		setInfo.bindingCount = std::size(setBinding.second);
		setInfo.pBindings = std::data(setBinding.second);
		vkn::error_check(vkCreateDescriptorSetLayout(device_.device, &setInfo, nullptr, &layout), "Failed to create the set");
		layouts.push_back(layout);
	}

	if (!std::empty(layouts))
	{
		VkDescriptorSetAllocateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		setInfo.pNext = nullptr;
		setInfo.descriptorPool = descriptorPool_;
		setInfo.descriptorSetCount = std::size(layouts);
		setInfo.pSetLayouts = std::data(layouts);

		sets_.resize(std::size(layouts));
		vkn::error_check(vkAllocateDescriptorSets(device_.device, &setInfo, std::data(sets_)), "Failed to allocate pipeline's sets");
	}
	return layouts;
}

void vkn::PipelineLayout::createPushConstantRanges(const std::vector<vkn::Shader>& shaders)
{
	for (const auto& shader : shaders)
	{
		const auto& pushConstants = shader.pushConstants();
		for (const auto& pushConstant : pushConstants)
		{
			assert(std::size(pushConstant.offsets) == std::size(pushConstant.ranges) && "The number of elements in the push constant is ambiguous");
			VkPushConstantRange range{};
			range.stageFlags = pushConstant.stageFlag;
			range.offset = pushConstant.offsets[0];
			range.size = pushConstant.size;
			pushConstantRanges_.push_back(range);
		}
	}
}
