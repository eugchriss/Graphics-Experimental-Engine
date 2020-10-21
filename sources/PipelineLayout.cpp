#include "../headers/PipelineLayout.h"
#include "../headers/vulkan_utils.h"

#include <unordered_map>

vkn::PipelineLayout::PipelineLayout(vkn::Device& device, const std::vector<vkn::Shader>& shaders) : device_{ device }
{
	createSets(shaders);
	createPushConstantRanges(shaders);

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0;
	layoutInfo.pushConstantRangeCount = std::size(pushConstantRanges_);
	layoutInfo.pPushConstantRanges = std::data(pushConstantRanges_);
	layoutInfo.setLayoutCount = std::size(sets_);
	layoutInfo.pSetLayouts = std::data(sets_);
	vkn::error_check(vkCreatePipelineLayout(device.device, &layoutInfo, nullptr, &layout), "Failed to create the pipeline Layout");
}

vkn::PipelineLayout::PipelineLayout(PipelineLayout&& other): device_{other.device_}
{
	layout = other.layout;
	sets_ = std::move(other.sets_);

	other.layout = VK_NULL_HANDLE;
}

vkn::PipelineLayout::~PipelineLayout()
{
	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device_.device, layout, nullptr);
	}
	if (!std::empty(sets_))
	{
		for (auto set : sets_)
		{
			vkDestroyDescriptorSetLayout(device_.device, set, nullptr);
		}
	}
}

const std::vector<VkDescriptorSetLayout>& vkn::PipelineLayout::layouts() const
{
	return sets_;
}

void vkn::PipelineLayout::createSets(const std::vector<vkn::Shader>& shaders)
{
	std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setBindings;
	for (const auto& shader : shaders)
	{
		auto& shaderBindings = shader.bindings();
		for (const auto& shaderBinding : shaderBindings)
		{
			setBindings[shaderBinding.set].push_back(shaderBinding.layoutBinding);
		}
	}

	for (const auto& setBinding : setBindings)
	{
		VkDescriptorSetLayout set{ VK_NULL_HANDLE };
		std::vector<VkDescriptorBindingFlags> bindingFlags;
		for (const auto binding : setBinding.second)
		{
			bindingFlags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
		}
		VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
		flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		flagsInfo.pNext = nullptr;
		flagsInfo.bindingCount = std::size(bindingFlags);
		flagsInfo.pBindingFlags = std::data(bindingFlags);

		VkDescriptorSetLayoutCreateInfo setInfo{};
		setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setInfo.pNext = &flagsInfo;
		setInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		setInfo.bindingCount = std::size(setBinding.second);
		setInfo.pBindings = std::data(setBinding.second);

		vkn::error_check(vkCreateDescriptorSetLayout(device_.device, &setInfo, nullptr, &set), "Failed to create the set");
		sets_.push_back(set);
	}
}

void vkn::PipelineLayout::createPushConstantRanges(const std::vector<vkn::Shader>& shaders)
{
	for (const auto& shader : shaders)
	{
		const auto& pushConstants = shader.pushConstants();
		for (const auto& pushConstant : pushConstants)
		{
			VkPushConstantRange range{};
			range.stageFlags = pushConstant.stageFlag;	
			range.offset = pushConstant.offset;
			range.size = pushConstant.size;
			pushConstantRanges_.push_back(range);
		}
	}
}
