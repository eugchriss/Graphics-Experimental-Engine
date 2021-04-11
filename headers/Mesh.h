#pragma once
#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "Vertex.h"
#include "material.h"
#include "assimp/material.h"

namespace gee
{

	struct Geometry
	{
		Geometry(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	class Mesh
	{
	public:
		Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
		~Mesh() = default;
		Mesh(Mesh&&) = default;
		const Geometry& geomtry() const;
		const std::vector<gee::Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const std::string& name() const;
		const size_t hash() const;

	private:
		std::string name_;
		Geometry geometry_;
		uint32_t rendererID_{};
	};
}

