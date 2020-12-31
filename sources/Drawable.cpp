#include "..\headers\Drawable.h"
#include "glm/gtx/transform.hpp"
#include <stdexcept>

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot) :
	mesh{ mesh },
	color{ col, 1.0f }, boundingBox_{ mesh.vertices() }
{
	name = std::string{ "Drawable : " } + std::to_string(count);
	setPosition(pos);
	setRotation(rot);
	scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size_ /= scaleFactor;
	count++;
}

gee::Drawable::Drawable(const gee::Mesh& mesh, const Optics& optics, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot) :
	mesh{ mesh }, light_{ optics },
	color{ col, 1.0f }, boundingBox_{ mesh.vertices() }
{
	setPosition(pos);
	setRotation(rot);
	light_->position = position_;
	light_->diffuse = color;
}

gee::Drawable::Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot) :
	name{ noun }, mesh{ mesh },
	color{ col, 1.0f }, boundingBox_{ mesh.vertices() }
{
	setPosition(pos);
	setRotation(rot);
	scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size_ /= scaleFactor;
}
gee::Drawable::Drawable(const std::string& noun, gee::Mesh&& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot) :
	name{ noun }, mesh{ std::move(mesh) },
	color{ col, 1.0f }, boundingBox_{ mesh.vertices() }
{
	setPosition(pos);
	setRotation(rot);
	scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size_ /= scaleFactor;
}
gee::Drawable::Drawable(const std::string& noun, const gee::Mesh& mesh, const Optics& optics, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot) :
	name{ noun }, mesh{ mesh }, light_{ optics },
	color{ col, 1.0f }, boundingBox_{ mesh.vertices() }
{
	setPosition(pos);
	setRotation(rot);
	light_->position = position_;
	light_->diffuse = color;
}

void gee::Drawable::setPosition(const glm::vec3& pos)
{
	if (position_ != pos)
	{
		shouldRecomputeTransform_ = true;
		position_ = pos;
		if (light_)
		{
			light_->position = position_;
		}
	}
}

void gee::Drawable::setColor(const glm::vec3& col)
{
	color = glm::vec4{ col, 1.0f };
	if (light_)
	{
		light_->diffuse = col;
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

		shouldRecomputeTransform_ = false;
	}
	return transform_;
}

bool gee::Drawable::hasLightComponent() const
{
	return light_.has_value();
}

const gee::BoundingBox& gee::Drawable::boundingBox() const
{
	return boundingBox_;
}

gee::Optics& gee::Drawable::light()
{
	assert(light_.has_value() && "This drawable is not a light source");
	return light_.value();
}

const gee::Optics& gee::Drawable::light() const
{
	assert(light_.has_value() && "This drawable is not a light source");
	return light_.value();
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
