#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "vertex.h"
#include "material.h"
#include "Texture.h"
#include "assimp/material.h"
#include "BoundingBox.h"

namespace gee
{

	class Mesh
	{
	public:
		Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, gee::Material&& material);
		~Mesh() = default;
		Mesh(Mesh&&) = default;
		const Material& material() const;
		const std::vector<Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const std::string& name() const;
		const size_t hash() const;

	private:
		std::string name_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;
		Material material_;
		uint32_t rendererID_{};
	};
}

