#include "../headers/Geometry.h"

gee::Geometry::Geometry(std::vector<Vertex>&& vertices_, std::vector<uint32_t>&& indices_) :
	vertices{ std::move(vertices_) }, indices{ std::move(indices_) }
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