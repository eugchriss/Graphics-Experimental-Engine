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

		enum class GLSL_Type
		{
			BOOL,
			UINT,
			INT,
			FLOAT
		};
		struct PushConstant
		{
			std::string name{};
			VkShaderStageFlagBits stageFlag{};
			std::vector<VkDeviceSize> offsets{};
			std::vector<VkDeviceSize> ranges{};
			uint32_t size{};
		};
		struct Tweaking
		{
			Tweaking() = default;
			Tweaking(Tweaking& t) = delete;
			Tweaking(Tweaking&&) = default;
			std::string name{};
			VkDeviceSize offset{};
			VkDeviceSize size{};
			GLSL_Type dataType{};
			float data{};
			float min{};
			float max{ 10.0f };
		};
		struct Attachment
		{
			uint32_t layoutIndex{};
			std::string name{};
			VkFormat format{};
			bool isPresentBuffer{ false };
		};

		const std::vector<Binding>& bindings() const;
		const std::vector<PushConstant>& pushConstants() const;
		std::vector<Tweaking>& tweakings();
		const std::vector<VkDescriptorPoolSize> poolSize() const;
		const std::pair<std::vector<VkVertexInputAttributeDescription>, uint32_t> attributeDescriptions() const;
		const std::vector<vkn::Shader::Attachment>& outputAttachments() const;
		const std::vector<std::string>& inputTexturesNames() const;
		const std::vector<vkn::Shader::Attachment>& subpassInputAttachments() const;
		static std::string tweakingName_;
	private:
		vkn::Device& device_;
		VkShaderModule module_{ VK_NULL_HANDLE };
		VkShaderStageFlagBits stage_;
		std::vector<Binding> bindings_;
		std::vector<PushConstant> pushConstants_;
		std::vector<Tweaking> tweakings_;
		std::vector<vkn::Shader::Attachment> outputAttachments_;
		std::vector<vkn::Shader::Attachment> subpassInputAttachments_;
		std::vector<std::string> inputTexturesNames_;
		std::unique_ptr<spirv_cross::CompilerGLSL> spirv_;
		const std::vector<char> readFile(const std::string& path);
		void getShaderResources(const std::string& path);
		void introspect(const VkShaderStageFlagBits stage);
		const Binding parseBinding(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage, const VkDescriptorType type);
		//Doesnt handle pushconstant with array yet
		const PushConstant parsePushConstant(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage);
		void getTweakingDataType(vkn::Shader::Tweaking& tweaking, const spirv_cross::SPIRType::BaseType& type);
	};

}