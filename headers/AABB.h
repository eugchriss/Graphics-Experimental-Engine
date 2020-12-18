#pragma once
#include "Vertex.h"
#include "utility.h"
#include "glm/glm.hpp"
#include <vector>
namespace gee
{
	class AABB
	{
	public:
		AABB(const std::vector<gee::Vertex>& vertices);
		const std::vector<gee::Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const gee::Mesh& mesh() const;
		glm::mat4 transformMatrix{1.0f};
	private:
		gee::Mesh mesh_;
		glm::vec3 origin_{};
		glm::vec3 extend_{};
	};

}
