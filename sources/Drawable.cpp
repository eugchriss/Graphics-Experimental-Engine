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
gee::Drawable::Drawable(const std::string& noun, gee::Mesh&& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot):
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
	const auto relativePos = pos - position_;
	if (relativePos != glm::vec3{ 0.0f })
	{
		position_ = pos;
		if (light_)
		{
			light_->position = position_;
		}
		transform = glm::translate(transform, relativePos);
	}
}

void gee::Drawable::setColor(const glm::vec3& col)
{
	color = glm::vec4{ col, 1.0f };
	light_->diffuse = col;
}

void gee::Drawable::setRotation(const glm::vec3& rot)
{
	const auto relativeRot = rot - rotation_;
	if (relativeRot != glm::vec3{ 0.0f })
	{
		rotation_ = rot;

		transform = glm::rotate(transform, relativeRot.x, glm::vec3{ 1.0, 0.0f, 0.0f });
		transform = glm::rotate(transform, relativeRot.y, glm::vec3{ 0.0, 1.0f, 0.0f });
		transform = glm::rotate(transform, relativeRot.z, glm::vec3{ 0.0, 0.0f, 1.0f });
	}
}

void gee::Drawable::setSize(const glm::vec3& size)
{
	auto relativeSize = size / size_;
	if (std::isinf(relativeSize.x))
		relativeSize.x = 1.0f;
	if (std::isinf(relativeSize.y))
		relativeSize.y = 1.0f;
	if (std::isinf(relativeSize.z))
		relativeSize.z = 1.0f;
	if (relativeSize != glm::vec3{ 1.0f })
	{
		size_ = size;
		transform = glm::scale(transform, relativeSize);
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
	if (light_.has_value())
	{
		return *light_;
	}
	else
	{
		throw std::runtime_error{ "This drawable has no optics component" };
	}
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
