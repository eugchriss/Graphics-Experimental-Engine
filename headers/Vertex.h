#pragma once
#include "glm/glm.hpp"

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
		glm::vec3 tangent{};
		glm::vec3 bitangent{};
	};
}
