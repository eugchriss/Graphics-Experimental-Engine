#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "vertex.h"
#include "mesh.h"
#include "utility.h"

namespace gee
{	
	class Mesh;
	class BoundingBox
	{
	public:
		BoundingBox(const std::vector<gee::Vertex>& vertices);
		BoundingBox(BoundingBox&&) = default;
		const std::vector<gee::Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const gee::Mesh& mesh() const;
		glm::mat4 transformMatrix{1.0f};
	private:
		gee::Mesh mesh_;
		float width_{};
		float height_{};
		float depth_{};

		struct Box
		{
			glm::vec3 origin{};
			glm::vec3 extend{};
		};

		const Box getBoxFromMesh(const std::vector<gee::Vertex>& vertices) const;
		void getTransformCubeMesh(const Box&);
	};
}
