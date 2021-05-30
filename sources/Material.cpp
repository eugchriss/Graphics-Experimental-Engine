#include "../headers/Material.h"

gee::Material::Material(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) : vertexShaderPath_{vertexShaderPath}, fragmentShaderPath_{fragmentShaderPath}
{
}

const std::string& gee::Material::vertexShaderPath() const
{
	return vertexShaderPath_;
}

const std::string& gee::Material::geometryShaderPath() const
{
	return geometryShaderPath_;
}

const std::string& gee::Material::fragmentShaderPath() const
{
	return fragmentShaderPath_;
}

void gee::Material::wireframe_on(const bool value)
{
	hasChanged_ = true;
	useWireFrame_ = value;
}
