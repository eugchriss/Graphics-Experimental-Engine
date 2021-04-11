#pragma once
#include "vulkan/vulkan.hpp"
#include "Device.h"
#include "Shader.h"

#include <vector>

namespace vkn
{
	class PipelineLayout
	{
	public:
		PipelineLayout(vkn::Device& device, const std::vector<vkn::Shader>& shaders);
		PipelineLayout(PipelineLayout&& other);
		~PipelineLayout();

		VkPipelineLayout layout{ VK_NULL_HANDLE };
		const std::vector<VkDescriptorSet>& sets() const;

	private:
		vkn::Device& device_;
		VkDescriptorPool descriptorPool_{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> sets_;
		std::vector<VkPushConstantRange> pushConstantRanges_;
		void create_descriptor_pool(const std::vector<Shader>& shaders);
		const std::vector<VkDescriptorSetLayout> createSets(const std::vector<vkn::Shader>& shaders);
		void createPushConstantRanges(const std::vector<vkn::Shader>& shaders);
	};
}