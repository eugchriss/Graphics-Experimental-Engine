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
		const std::vector<VkDescriptorSetLayout>& layouts() const;
		const std::vector<VkDescriptorSetLayout>& subpassInputLayouts() const;

	private:
		vkn::Device& device_;
		std::vector<VkDescriptorSetLayout> sets_;
		std::vector<VkDescriptorSetLayout> subpassInputSets_;
		std::vector<VkPushConstantRange> pushConstantRanges_;
		void createSets(const std::vector<vkn::Shader>& shaders);
		void createSubpassInputSets(const std::vector<vkn::Shader>& shaders);
		void createPushConstantRanges(const std::vector<vkn::Shader>& shaders);
	};
}