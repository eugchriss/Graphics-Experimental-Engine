#include "..\headers\BoundingBox.h"
#include <algorithm>
#include "glm/gtx/transform.hpp"

gee::BoundingBox::BoundingBox(const std::vector<gee::Vertex>& vertices) : mesh_{gee::getCubeMesh()}
{
	auto box = getBoxFromMesh(vertices);
	width_ = box.extend.x - box.origin.x;
	height_ = box.extend.y - box.origin.y;
	depth_ = box.extend.z - box.origin.z;
	if (width_ == 0.0f)
	{
		width_ = 0.00002f;
	}
	if (height_ == 0.0f)
	{
		height_ = 0.00002f;
	}
	if (depth_ == 0.0f)
	{
		depth_ = 0.00002f;
	}

	getTransformCubeMesh(box);
}

const std::vector<gee::Vertex>& gee::BoundingBox::vertices() const
{
	return mesh_.vertices();
}

const std::vector<uint32_t>& gee::BoundingBox::indices() const
{
	return mesh_.indices();
}

const gee::Mesh& gee::BoundingBox::mesh() const
{
	return mesh_;
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
	scale.x = width_ / 2.0f;
	scale.y = height_ / 2.0f;
	scale.z = depth_ / 2.0f;

	auto offset = 0.5f * (box.origin + box.extend);
	transformMatrix = glm::translate(transformMatrix, offset);
	transformMatrix = glm::scale(transformMatrix, scale);
}