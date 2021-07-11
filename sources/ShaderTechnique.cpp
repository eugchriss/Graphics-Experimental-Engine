#include "../headers/ShaderTechnique.h"
#include <iterator>

gee::ShaderTechnique::ShaderTechnique(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) : vertexShaderPath_{ vertexShaderPath }, fragmentShaderPath_{ fragmentShaderPath }
{
}

const std::string& gee::ShaderTechnique::vertexShaderPath() const
{
	return vertexShaderPath_;
}

const std::string& gee::ShaderTechnique::geometryShaderPath() const
{
	return geometryShaderPath_;
}

const std::string& gee::ShaderTechnique::fragmentShaderPath() const
{
	return fragmentShaderPath_;
}

void gee::ShaderTechnique::set_dynamic_alignments(const Set set, const Alignments& offsets)
{
	if (!isGPUInitialized_)
	{
		setDynamicAlignments_[set] = offsets;
	}
	else
	{
		throw std::runtime_error{ "can t modify a running shader technique" };
	}
}

const std::unordered_map<gee::ShaderTechnique::Set, gee::ShaderTechnique::Alignments>& gee::ShaderTechnique::dynamic_alignments() const
{
	return setDynamicAlignments_;
}