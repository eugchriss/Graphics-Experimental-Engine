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

		glm::mat4 transformMatrix{1.0f};
	private:
		gee::Cube mesh;
		float width{};
		float height{};
		float depth{};

		struct Box
		{
			glm::vec3 origin{};
			glm::vec3 extend{};
		};

		const Box getBoxFromMesh(const std::vector<gee::Vertex>& vertices) const;
		void getTransformCubeMesh(const Box&);
		float map(const float x, const float initialRangeBegin, const float initialRangeEnd, const float finalRangeBegin, const float finalRangeEnd);
	};
}
