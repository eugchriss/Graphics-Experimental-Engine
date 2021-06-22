#pragma once
#include <string>
#include <vector>

#include "geometry.h"
#include "assimp/material.h"
#include "glm/glm.hpp"
#include "impl/vulkan/vulkan_utils.h"

namespace gee
{
	class Mesh
	{
	public:
		Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
		~Mesh() = default;
		Mesh(Mesh&&) = default;
		Geometry& geometry();
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