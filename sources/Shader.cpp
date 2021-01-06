#include "../headers/Shader.h"

#include <fstream>

std::string vkn::Shader::tweakingName_{ "tweakings" };
vkn::Shader::Shader(vkn::Device& device, const VkShaderStageFlagBits stage, const std::string& path) : device_{ device }, stage_{ stage }
{
	auto spirv = readFile(path);

	VkShaderModuleCreateInfo moduleInfo{};
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = std::size(spirv);
	moduleInfo.pCode = reinterpret_cast<uint32_t*>(std::data(spirv));

	vkn::error_check(vkCreateShaderModule(device_.device, &moduleInfo, nullptr, &module_), "Failed to create the shader module");

	getShaderResources(path);
	introspect(stage);
}

vkn::Shader::Shader(Shader&& other) : device_{ other.device_ }
{
	module_ = other.module_;
	stage_ = other.stage_;
	/*not neccesary but for rigor	*/
	bindings_ = std::move(other.bindings_);
	pushConstants_ = std::move(other.pushConstants_);
	outputAttachments_ = std::move(other.outputAttachments_);
	tweakings_ = std::move(other.tweakings_);
	spirv_ = std::move(other.spirv_);
	/*****/

	other.module_ = VK_NULL_HANDLE;
}

vkn::Shader::~Shader()
{
	if (module_ != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device_.device, module_, nullptr);
	}
}

const VkShaderModule vkn::Shader::module() const
{
	return module_;
}

const VkShaderStageFlagBits vkn::Shader::stage() const
{
	return stage_;
}

const std::vector<vkn::Shader::Binding>& vkn::Shader::bindings() const
{
	return bindings_;
}

const std::vector<vkn::Shader::PushConstant>& vkn::Shader::pushConstants() const
{
	return pushConstants_;
}

std::vector<vkn::Shader::Tweaking>& vkn::Shader::tweakings()
{
	return tweakings_;
}

const std::vector<VkDescriptorPoolSize> vkn::Shader::poolSize() const
{
	auto& resources = spirv_->get_shader_resources();
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , static_cast<uint32_t>(std::size(resources.uniform_buffers)) + 1 });
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , static_cast<uint32_t>(std::size(resources.sampled_images)) + 10000 });

	return poolSizes;
}

const std::pair<std::vector<VkVertexInputAttributeDescription>, uint32_t> vkn::Shader::attributeDescriptions() const
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	uint32_t offset{ 0 };
	auto resources = spirv_->get_shader_resources();
	auto inputResources = resources.stage_inputs;
	std::sort(std::begin(inputResources), std::end(inputResources), [&](const auto& lhs, const auto& rhs) {return spirv_->get_decoration(lhs.id, spv::DecorationLocation) < spirv_->get_decoration(rhs.id, spv::DecorationLocation); });
	for (const auto& resource : inputResources)
	{
		auto format = vkn::getFormat(spirv_->get_type(resource.base_type_id));
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = spirv_->get_decoration(resource.id, spv::DecorationBinding);
		attributeDescription.location = spirv_->get_decoration(resource.id, spv::DecorationLocation);
		attributeDescription.format = format.format;
		attributeDescription.offset = offset;

		attributeDescriptions.push_back(attributeDescription);

		offset += format.range;
	}
	return std::make_pair(attributeDescriptions, offset);
}

const std::vector<vkn::Shader::Attachment>& vkn::Shader::outputAttachments() const
{
	return outputAttachments_;
}

const std::vector<std::string>& vkn::Shader::inputTexturesNames() const
{
	return inputTexturesNames_;
}

const std::vector<vkn::Shader::Attachment>& vkn::Shader::subpassInputAttachments() const
{
	return subpassInputAttachments_;
}

const std::vector<char> vkn::Shader::readFile(const std::string& path)
{
	std::ifstream file{ path, std::ios::ate | std::ios::binary };
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	auto size = file.tellg();
	file.seekg(0);
	std::vector<char> datas(size);
	file.read(std::data(datas), size);

	return datas;
}

void vkn::Shader::getShaderResources(const std::string& path)
{
	std::ifstream file{ path, std::ios::ate | std::ios::in | std::ios::binary };
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	auto size = file.tellg();
	file.seekg(0);
	std::vector<uint32_t> spirv(size / sizeof(uint32_t));
	file.read(reinterpret_cast<char*>(std::data(spirv)), size);

	spirv_ = std::make_unique<spirv_cross::CompilerGLSL>(std::move(spirv));
}

void vkn::Shader::introspect(const VkShaderStageFlagBits stage)
{
	auto& resources = spirv_->get_shader_resources();

	for (const auto& resource : resources.uniform_buffers)
	{
		bindings_.push_back(parseBinding(resource, stage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
	}
	for (const auto& resource : resources.sampled_images)
	{
		inputTexturesNames_.emplace_back(resource.name);
		bindings_.push_back(parseBinding(resource, stage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
	}
	for (const auto& resource : resources.push_constant_buffers)
	{
		//Tweakings parsing only supports primitives types. No vectors or matrices.
		pushConstants_.push_back(parsePushConstant(resource, stage_));
	}
	for (const auto& resource : resources.stage_outputs)
	{
		auto spirvType = spirv_->get_type(resource.type_id);
		Attachment attchment{};
		attchment.layoutIndex = spirv_->get_decoration(resource.id, spv::DecorationLocation);
		attchment.name = resource.name;
		attchment.format = vkn::getFormat(spirvType).format;
		outputAttachments_.emplace_back(attchment);
	}
	for (const auto& resource : resources.subpass_inputs)
	{
		auto spirvType = spirv_->get_type(resource.type_id);
		Attachment attchment{};
		attchment.layoutIndex = spirv_->get_decoration(resource.id, spv::DecorationLocation);
		attchment.name = resource.name;
		attchment.format = vkn::getFormat(spirvType).format;
		subpassInputAttachments_.emplace_back(attchment);
	}
}

const vkn::Shader::Binding vkn::Shader::parseBinding(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage, const VkDescriptorType type)
{
	auto spirvType = spirv_->get_type(resource.type_id);

	size_t range{};
	if (spirvType.basetype == spirv_cross::SPIRType::Struct)
	{
		range = spirv_->get_declared_struct_size(spirvType);
	}

	uint32_t size{ 1 };
	if (std::size(spirvType.array) > 0)
	{
		for (auto i = 0u; i < std::size(spirvType.array); ++i)
		{
			size *= spirvType.array[i];
		}
	}

	VkDescriptorSetLayoutBinding descriptorBinding;
	descriptorBinding.binding = spirv_->get_decoration(resource.id, spv::DecorationBinding);
	descriptorBinding.stageFlags = stage;
	descriptorBinding.descriptorType = type;
	descriptorBinding.descriptorCount = size;
	descriptorBinding.pImmutableSamplers = nullptr;

	Binding binding;
	binding.name = resource.name;
	binding.set = spirv_->get_decoration(resource.id, spv::DecorationDescriptorSet);
	binding.layoutBinding = descriptorBinding;
	binding.size = size;
	if ((spirvType.basetype != spirv_cross::SPIRType::Struct) && (spirvType.basetype != spirv_cross::SPIRType::SampledImage))
	{
		binding.format = vkn::getFormat(spirvType);
	}
	binding.range = range;
	return binding;
}

const vkn::Shader::PushConstant vkn::Shader::parsePushConstant(const spirv_cross::Resource& resource, const VkShaderStageFlagBits stage)
{
	auto resourceType = spirv_->get_type(resource.base_type_id);
	auto pushConstantRanges = spirv_->get_active_buffer_ranges(resource.id);

	PushConstant pc{};
	pc.name = resource.name;
	pc.stageFlag = stage_;
	for (auto i = 0u; i < std::size(pushConstantRanges); ++i)
	{
		pc.offsets.push_back(pushConstantRanges[i].offset);
		pc.ranges.push_back(pushConstantRanges[i].range);
		pc.size += pushConstantRanges[i].range;
	}
	assert(std::size(pc.offsets) && "The push constant is not ACTIVELY used by the shader");
	if (resource.name == tweakingName_)
	{
		assert(std::size(resourceType.array) == 0 && "Tweaking should only be primitives type (AKA no veci or mati)");

		const auto member_count = resourceType.member_types.size();
		for (auto i = 0u; i < member_count; ++i)
		{
			auto tweakingType = spirv_->get_type(resourceType.member_types[i]);
			auto& tweaking = tweakings_.emplace_back(Tweaking{});
			tweaking.name = spirv_->get_member_name(resourceType.self, i);
			tweaking.size = spirv_->get_declared_struct_member_size(resourceType, i);
			tweaking.offset = spirv_->type_struct_member_offset(resourceType, i);
			getTweakingDataType(tweaking, tweakingType.basetype);
		}
	}
	return pc;
}

void vkn::Shader::getTweakingDataType(vkn::Shader::Tweaking& tweaking, const spirv_cross::SPIRType::BaseType& type)
{
	switch (type)
	{
	case spirv_cross::SPIRType::BaseType::UInt:
		tweaking.dataType = Shader::GLSL_Type::UINT;
		break;
	case spirv_cross::SPIRType::BaseType::Int:
		tweaking.dataType = Shader::GLSL_Type::INT;
		break;
	case spirv_cross::SPIRType::BaseType::Float:
		tweaking.dataType = Shader::GLSL_Type::FLOAT;
		break;
	default:
		throw std::runtime_error{ "untreated type" };
		break;
	}
}
