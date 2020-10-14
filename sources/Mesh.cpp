#include <stdexcept>
#include "../headers/Mesh.h"

gee::Mesh::Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::unordered_map<aiTextureType, Texture>&& textures) :
	name_{ name }, vertices_{ std::move(vertices) },
	indices_{ std::move(indices) }, textures_{std::move(textures)}
{
}
const gee::Texture& gee::Mesh::texture(const aiTextureType& type) const
{
	auto result = textures_.find(type);
	if (result != std::end(textures_))
	{
		return result->second;
	}
	else
	{
		throw std::runtime_error{ "The mesh doesn t provide the requested texture type" };
	}

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
