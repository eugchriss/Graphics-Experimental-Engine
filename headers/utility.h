#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "AbstractMesh.h"

namespace gee
{
	class Cube : public AbstractMesh
	{
	public:
		virtual const std::vector<gee::Vertex>& vertices() const final;
		virtual const std::vector<uint32_t>& indices() const final;
		std::vector<gee::Vertex> vertices_
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
		std::vector<uint32_t> indices_{
			0, 1, 3, 1, 2, 3, //front
			4, 5, 7, 5, 6, 7, //back
			4, 0, 7, 0, 3, 7, //left
			1, 5, 2, 5, 6, 2, //right 
			4, 5, 0, 5, 1, 0, //up
			7, 6, 3, 6, 2, 3 // bottom
		};
	};

	class QuadMesh : public AbstractMesh
	{
	public:
		virtual const std::vector<gee::Vertex>& vertices() const final;
		virtual const std::vector<uint32_t>& indices() const  final;
		std::vector<gee::Vertex> vertices_
		{
			gee::Vertex{glm::vec3{-1.0f, 1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}},
			gee::Vertex{glm::vec3{1.0f, 1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
			gee::Vertex{glm::vec3{1.0f, -1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
			gee::Vertex{glm::vec3{-1.0f, -1.0f, 0.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}}
		};
		std::vector<uint32_t> indices_{ 0, 1, 3, 1, 2, 3 };
	};

	class Floor : public AbstractMesh
	{
	public:
		Floor();
		virtual const std::vector<gee::Vertex>& vertices() const final;
		virtual const std::vector<uint32_t>& indices() const final;
		std::vector<gee::Vertex> vertices_
		{
			gee::Vertex{glm::vec3{-1.0f, 0.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 0.0f}},
			gee::Vertex{glm::vec3{1.0f, 0.0f, -1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 0.0f}},
			gee::Vertex{glm::vec3{1.0f, 0.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{1.0f, 1.0f}},
			gee::Vertex{glm::vec3{-1.0f, 0.0f, 1.0f}, glm::vec3{1.0f}, glm::vec2{0.0f, 1.0f}}
		};
		std::vector<uint32_t> indices_{ 0, 1, 3, 1, 2, 3 };
	};
}