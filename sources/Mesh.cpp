#include <stdexcept>
#include "../headers/Mesh.h"
#include "../headers/utility.h"

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


gee::Vertex::Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& tex, const glm::vec3& norm):
	position{ pos }, color{col},
	textureCoord{tex}, normal{norm}
{
}

gee::Geometry::Geometry(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices):
	vertices {std::move(vertices)}, indices{ std::move(indices) }
{
	gee::hash_combine(hash, std::size(vertices), std::size(indices));
}
gee::Geometry gee::getCubeGeometry()
{
	std::vector<gee::Vertex> vertices
	{
		gee::Vertex{glm::vec3{-1.0f, 1.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, 1.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, -1.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
		gee::Vertex{glm::vec3{-1.0f, -1.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}},
		gee::Vertex{glm::vec3{-1.0f, 1.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, 1.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, -1.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
		gee::Vertex{glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}}
	};
	std::vector<uint32_t> indices{
			0, 1, 3, 1, 2, 3, //front
			4, 5, 7, 5, 6, 7, //back
			4, 0, 7, 0, 3, 7, //left
			1, 5, 2, 5, 6, 2, //right 
			4, 5, 0, 5, 1, 0, //up
			7, 6, 3, 6, 2, 3 // bottom
	};

	gee::processTangentSpace(vertices, indices);
	return { std::move(vertices), std::move(indices) };
}

gee::Geometry gee::getQuadGeometry()
{
	std::vector<gee::Vertex> vertices
	{
		gee::Vertex{glm::vec3{-1.0f, 1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, 1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
		gee::Vertex{glm::vec3{1.0f, -1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
		gee::Vertex{glm::vec3{-1.0f, -1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}}
	};
	std::vector<uint32_t> indices{ 0, 1, 3, 1, 2, 3 };
	gee::processTangentSpace(vertices, indices);

	return { std::move(vertices), std::move(indices) };
}

gee::Geometry gee::getFloorGeometry()
{
	std::vector<gee::Vertex> vertices
	{
		gee::Vertex{glm::vec3{-1.0f, 0.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}},
		gee::Vertex{glm::vec3{1.0f, 0.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
		gee::Vertex{glm::vec3{1.0f, 0.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
		gee::Vertex{glm::vec3{-1.0f, 0.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}}
	};
	std::vector<uint32_t> indices{ 0, 1, 3, 1, 2, 3 };
	gee::processTangentSpace(vertices, indices);

	return { std::move(vertices), std::move(indices) };
}