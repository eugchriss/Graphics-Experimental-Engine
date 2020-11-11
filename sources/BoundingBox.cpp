#include "..\headers\BoundingBox.h"
#include <algorithm>
#include "glm/gtx/transform.hpp"
gee::BoundingBox::BoundingBox(const std::vector<gee::Vertex>& vertices)
{
	auto box = getBoxFromMesh(vertices);
	width = box.extend.x - box.origin.x;
	height = box.extend.y - box.origin.y;
	depth = box.extend.z - box.origin.z;
	if (width == 0.0f)
	{
		width = 0.00002f;
	}
	if (height == 0.0f)
	{
		height = 0.00002f;
	}
	if (depth == 0.0f)
	{
		depth = 0.00002f;
	}

	getTransformCubeMesh(box);
}

const std::vector<gee::Vertex>& gee::BoundingBox::vertices() const
{
	return mesh.vertices;
}

const std::vector<uint32_t>& gee::BoundingBox::indices() const
{
	return mesh.indices;
}

const gee::BoundingBox::Box gee::BoundingBox::getBoxFromMesh(const std::vector<gee::Vertex>& vertices) const
{
	Box box{};
	for (const auto& vertex : vertices)
	{
		if (vertex.position.x < box.origin.x)
		{
			box.origin.x = vertex.position.x;
		}
		if (vertex.position.y < box.origin.y)
		{
			box.origin.y = vertex.position.y;
		}
		if (vertex.position.z < box.origin.z)
		{
			box.origin.z = vertex.position.z;
		}

		if (vertex.position.x > box.extend.x)
		{
			box.extend.x = vertex.position.x;
		}
		if (vertex.position.y > box.extend.y)
		{
			box.extend.y = vertex.position.y;
		}
		if (vertex.position.z > box.extend.z)
		{
			box.extend.z = vertex.position.z;
		}
	}
	return box;
}

void gee::BoundingBox::getTransformCubeMesh(const Box& box)
{
	glm::vec3 scale{1.0f};
	scale.x = width / 2.0f;
	scale.y = height / 2.0f;
	scale.z = depth / 2.0f;

	auto offset = 0.5f * (box.origin + box.extend);
	transformMatrix = glm::translate(transformMatrix, offset);
	transformMatrix = glm::scale(transformMatrix, scale);
}

float gee::BoundingBox::map(const float x, const float initialRangeBegin, const float initialRangeEnd, const float finalRangeBegin, const float finalRangeEnd)
{
	return finalRangeBegin + ((finalRangeEnd - finalRangeBegin) / (initialRangeEnd - initialRangeBegin)) * (x - initialRangeBegin);
}
