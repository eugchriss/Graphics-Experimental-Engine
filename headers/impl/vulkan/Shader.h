#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include "Device.h"
#include "spirv_glsl.hpp"
#include "vulkan_utils.h"
#include "vulkan/vulkan.hpp"

namespace gee
{
	namespace vkn
	{
		const uint32_t PER_MATERIAL_SET = 0;
		class Shader
		{
		public:
			Shader(vkn::Device& device, const VkShaderStageFlagBits stage, const std::string& path, bool allowDynamicUniform = true);
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
			const std::vector<Binding>& subpassInputBindings() const;
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
			std::vector<Binding> subpassInputBindings_;
			std::vector<PushConstant> pushConstants_;
			std::vector<Attachment> outputAttachments_;
			std::unique_ptr<spirv_cross::CompilerGLSL> spirv_;
			bool allowDynamicUniform_;
			const std::vector<char> readFile(const std::string& path);
			void getShaderResources(const std::string& path);
			void introspect(const VkShaderStageFlagBits stage);
			const Binding parseBinding(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage, const VkDescriptorType type);
			const PushConstant parsePushConstant(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage);
		};
	}
}