#pragma once
#include "vulkan/vulkan.hpp"
#include "spirv_glsl.hpp"
#include "vulkan_utils.h"
#include "Device.h"

#include <string>
#include <vector>
#include <memory>
#include <variant>

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
		struct Attachment
		{
			uint32_t layoutIndex{};
			std::string name{};
			VkFormat format{};
		};
		struct PushConstant
		{
			std::string name{};
			VkShaderStageFlagBits stageFlag{};
			std::vector<VkDeviceSize> offsets{};
			std::vector<VkDeviceSize> ranges{};
			uint32_t size{};
		};

		const std::vector<Binding>& bindings() const;
		const std::vector<PushConstant>& pushConstants() const;
		const std::vector<VkDescriptorPoolSize> poolSize() const;
		const std::pair<std::vector<VkVertexInputAttributeDescription>, uint32_t> attributeDescriptions() const;
		const std::vector<Attachment>& outputAttachments() const;
		const std::vector<std::string>& inputTexturesNames() const;
	private:
		vkn::Device& device_;
		VkShaderModule module_{ VK_NULL_HANDLE };
		VkShaderStageFlagBits stage_;
		std::vector<std::string> inputTexturesNames_;
		std::vector<Binding> bindings_;
		std::vector<PushConstant> pushConstants_;
		std::vector<Attachment> outputAttachments_;
		std::unique_ptr<spirv_cross::CompilerGLSL> spirv_;
		const std::vector<char> readFile(const std::string& path);
		void getShaderResources(const std::string& path);
		void introspect(const VkShaderStageFlagBits stage);
		const Binding parseBinding(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage, const VkDescriptorType type);
		const PushConstant parsePushConstant(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage);
	};

}