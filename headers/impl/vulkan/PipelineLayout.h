#pragma once
#include <vector>

#include "Device.h"
#include "Shader.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
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
			std::vector<VkDescriptorSetLayout> setLayouts_;
			std::vector<VkDescriptorSet> sets_;
			std::vector<VkPushConstantRange> pushConstantRanges_;
			void create_descriptor_pool(const std::vector<Shader>& shaders);
			void createSets(const std::vector<vkn::Shader>& shaders);
			void createPushConstantRanges(const std::vector<vkn::Shader>& shaders);
		};
	}
}