#include <stdexcept>
#include "../headers/Mesh.h"

gee::Mesh::Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices) :
	name_{ name }, geometry_{std::move(vertices), std::move(indices) }
{
}

const gee::Geometry& gee::Mesh::geometry() const
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


gee::Vertex::Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& tex, const glm::vec3& norm):
	position{ pos }, color{col},
	textureCoord{tex}, normal{norm}
{
}

gee::Geometry::Geometry(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices):
	vertices {std::move(vertices)}, indices{ std::move(indices) }
{
}
