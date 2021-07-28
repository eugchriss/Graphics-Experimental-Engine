#include <stdexcept>
#include "../headers/Drawable.h"
#include "glm/gtx/transform.hpp"

gee::IdDispenser<size_t> gee::Drawable::idDispenser_{};

gee::Drawable::Drawable(const gee::Geometry& geometry, gee::Material& mat, const glm::vec3& pos, const glm::vec3& rot) 
 : geometry{geometry}, material{ mat }, position{pos}, rotation(rot)
{
	id = idDispenser_.get();

	//scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size /= scaleFactor;
}

gee::Drawable::Drawable(Drawable&& other): geometry{std::move(other.geometry)}, material{other.material}
{
	other.shouldDeleteId_ = false;
	position = other.position;
	rotation = other.rotation;
	size = other.size;
	scaleFactor = other.scaleFactor;
	lastPosition_ = other.lastPosition_;
	lastRotation_ = other.lastRotation_;
	lastSize_ = other.lastSize_;
	id = other.id;
}

gee::Drawable::~Drawable()
{
	if (shouldDeleteId_)
	{
		idDispenser_.consumed(id);
	}
}

const glm::mat4& gee::Drawable::getTransform()
{
	if (position != lastPosition_)
	{
		if (position.x != lastPosition_.x)
		{
			transform_.translate(glm::vec3{ position.x - lastPosition_.x, 0.0f, 0.0f});
		}
		if (position.y != lastPosition_.y)
		{
			transform_.translate(glm::vec3{ 0.0f, position.y - lastPosition_.y, 0.0f});
		}
		if (position.z != lastPosition_.z)
		{
			transform_.translate(glm::vec3{ 0.0f, 0.0f, position.z - lastPosition_.z });
		}
		lastPosition_ = position;
	}
	if (size != lastSize_)
	{
		transform_.scale(size);
		lastSize_ = size;
	}
	if (rotation != lastRotation_)
	{
		transform_.rotate(rotation.x, glm::vec3{ 1.0, 0.0f, 0.0f });
		transform_.rotate(rotation.y, glm::vec3{ 0.0, 1.0f, 0.0f });
		transform_.rotate(rotation.z, glm::vec3{ 0.0, 0.0f, 1.0f });

		lastRotation_ = rotation;
	}
	return transform_.value();
}

bool gee::Drawable::operator==(const Drawable& other) const 
{
	return id == other.id;
}

const float gee::Drawable::normalizedScaleFactor(const gee::Mesh& mesh)
{
	float minX{}, maxX{};
	float minY{}, maxY{};
	float minZ{}, maxZ{};

	for (const auto& vertex : mesh.vertices())
	{
		minX = std::min(vertex.position.x, minX);
		minY = std::min(vertex.position.y, minY);
		minZ = std::min(vertex.position.z, minZ);

		maxX = std::max(vertex.position.x, maxX);
		maxY = std::max(vertex.position.y, maxY);
		maxZ = std::max(vertex.position.z, maxZ);
	}
	const auto x = maxX - minX;
	const auto y = maxY - minY;
	const auto z = maxZ - minZ;

	const auto volume = x * y * z;
	size = glm::vec3{ x, y, z };
	return glm::pow(volume, 1 / 3.0f);
}
