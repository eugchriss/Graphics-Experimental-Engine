#include <stdexcept>
#include "../headers/Mesh.h"

gee::Mesh::Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, gee::Material&& material) :
	name_{ name }, vertices_{ std::move(vertices) },
	indices_{ std::move(indices) }, material_{std::move(material)}
{
}
const gee::Material& gee::Mesh::material() const
{
	return material_;
}
const std::vector<gee::Vertex>& gee::Mesh::vertices() const
{
	return vertices_;
}

const std::vector<uint32_t>& gee::Mesh::indices() const
{
	return indices_;
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
