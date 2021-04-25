#include "..\headers\Drawable.h"
#include "glm/gtx/transform.hpp"
#include <stdexcept>

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos, const glm::vec3& rot)
	: Drawable{ std::string{ "Drawable : " } + std::to_string(count), geometry, materialInstance, pos, rot }
{
}

gee::Drawable::Drawable(const std::string& name, const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos, const glm::vec3& rot) 
 : geometry{geometry}, materialInstance{ materialInstance }
{
	setPosition(pos);
	setRotation(rot);
	//scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size_ /= scaleFactor;
}

void gee::Drawable::setPosition(const glm::vec3& pos)
{
	if (position_ != pos)
	{
		shouldRecomputeTransform_ = true;
		position_ = pos;
	}
}

void gee::Drawable::setRotation(const glm::vec3& rot)
{
	if (rotation_ != rot)
	{
		shouldRecomputeTransform_ = true;
		rotation_ = rot;
	}
}

void gee::Drawable::setSize(const glm::vec3& size)
{
	if (size_ != size)
	{
		shouldRecomputeTransform_ = true;
		size_ = size;
	}	
}

const glm::vec3& gee::Drawable::getPosition() const
{
	return position_;
}

const glm::vec3& gee::Drawable::getSize() const
{
	return size_;
}

const glm::vec3& gee::Drawable::getRotation() const
{
	return rotation_;
}

const glm::mat4& gee::Drawable::getTransform()
{
	if (shouldRecomputeTransform_)
	{
		transform_ = glm::mat4{ 1.0f };
		transform_ = glm::translate(transform_, position_);
		transform_ = glm::scale(transform_, size_);
		transform_ = glm::rotate(transform_, rotation_.x, glm::vec3{ 1.0, 0.0f, 0.0f });
		transform_ = glm::rotate(transform_, rotation_.y, glm::vec3{ 0.0, 1.0f, 0.0f });
		transform_ = glm::rotate(transform_, rotation_.z, glm::vec3{ 0.0, 0.0f, 1.0f });

		normalMatrix_ = glm::transpose(glm::inverse(transform_));
		shouldRecomputeTransform_ = false;
	}
	return transform_;
}

const glm::mat4& gee::Drawable::getNormalMatrix() const
{
	return normalMatrix_;
}

const size_t gee::Drawable::hash() const
{
	return std::hash<std::string>{}(name);
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
	size_ = glm::vec3{ x, y, z };
	return glm::pow(volume, 1 / 3.0f);
}
