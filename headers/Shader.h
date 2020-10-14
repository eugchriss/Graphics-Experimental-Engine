#pragma once
#include "vulkan/vulkan.hpp"
#include "spirv_glsl.hpp"
#include "vulkan_utils.h"
#include "Device.h"

#include <string>
#include <vector>
#include <memory>

namespace vkn
{
	class Shader
	{
	public:
		Shader(vkn::Device& device, const VkShaderStageFlagBits stage, const std::string& path);
		Shader(Shader&& other);
		~Shader();

		const VkShaderModule module() const;
		const VkShaderStageFlagBits stage() const;
		struct Binding
		{
			std::string name{};
			uint32_t set{};
			VkDescriptorSetLayoutBinding layoutBinding{};
			VkDeviceSize size{};
			VkDeviceSize range{};
			vkn::Format format;
		};

		struct PushConstant
		{
			std::string name{};
			VkShaderStageFlagBits stageFlag{};
			VkDeviceSize offset{};
			VkDeviceSize size{};
		};
		const std::vector<Binding>& bindings() const;
		const std::vector<PushConstant>& pushConstants() const;
		const std::vector<VkDescriptorPoolSize> poolSize() const;
		const std::pair<std::vector<VkVertexInputAttributeDescription>, uint32_t> attributeDescriptions() const;
		uint32_t outputCount{};
	private:
		vkn::Device& device_;
		VkShaderModule module_{ VK_NULL_HANDLE };
		VkShaderStageFlagBits stage_;
		std::vector<Binding> bindings_;
		std::vector<PushConstant> pushConstants_;
		std::unique_ptr<spirv_cross::CompilerGLSL> spirv_;

		const std::vector<char> readFile(const std::string& path);
		void getShaderResources(const std::string& path);
		void introspect(const VkShaderStageFlagBits stage);
		const Binding parseBinding(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage, const VkDescriptorType type);
		const PushConstant parsePushConstant(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage);
	};
}