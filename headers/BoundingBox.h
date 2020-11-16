#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "utility.h"

namespace gee
{	
	class BoundingBox
	{
	public:
		BoundingBox(const std::vector<gee::Vertex>& vertices);
		const std::vector<gee::Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const gee::Cube& mesh() const;
		glm::mat4 transformMatrix{1.0f};
	private:
		gee::Cube mesh_;
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
