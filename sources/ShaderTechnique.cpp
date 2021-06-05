#include "../headers/ShaderTechnique.h"

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

void gee::ShaderTechnique::wireframe_on(const bool value)
{
	hasChanged_ = true;
	useWireFrame_ = value;
}
