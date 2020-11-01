#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "material.h"
#include "Texture.h"
#include "assimp/material.h"
namespace gee
{
	struct Vertex
	{
		Vertex() = default;
		Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& tex, const glm::vec3& norm = glm::vec3{});
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec2 textureCoord{};
		glm::vec3 normal{};
	};
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

