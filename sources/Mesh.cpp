#include <stdexcept>
#include "../headers/Mesh.h"

gee::Mesh::Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices) :
	name_{ name }, geometry_{std::move(vertices), std::move(indices) }
{
}

gee::Geometry& gee::Mesh::geometry()
{
	return geometry_;
}
const std::vector<gee::Vertex>& gee::Mesh::vertices() const
{
	return geometry_.vertices;
}

const std::vector<uint32_t>& gee::Mesh::indices() const
{
	return geometry_.indices;
}

const std::string& gee::Mesh::name() const
{
	return name_;
}

const size_t gee::Mesh::hash() const
{
	return std::hash<std::string>{}(name_);
}