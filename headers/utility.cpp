#include "utility.h"

gee::Floor::Floor()
{
	vertices[0].tangent = glm::vec3{0.0f, 0.0f, 1.0f};
	vertices[1].tangent = glm::vec3{-1.0f, 0.0f, 0.0f};
	vertices[2].tangent = glm::vec3{0.0f, 0.0f, -1.0f};
	vertices[3].tangent = glm::vec3{1.0f, 0.0f, 0.0f};
	
	for (auto& vertex : vertices)
	{
		vertex.normal = glm::vec3{0.0f, 1.0f, 0.0f};
		vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
	}
}
