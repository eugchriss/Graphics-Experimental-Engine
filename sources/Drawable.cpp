#include <stdexcept>
#include "../headers/Drawable.h"
#include "glm/gtx/transform.hpp"

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos, const glm::vec3& rot)
	: Drawable{ std::string{ "Drawable : " } + std::to_string(count), geometry, materialInstance, pos, rot }
{
}

gee::Drawable::Drawable(const std::string& name, const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos, const glm::vec3& rot) 
 : geometry{geometry}, materialInstance{ materialInstance }, position{pos}, rotation(rot)
{

	//scaleFactor = normalizedScaleFactor(mesh);
	if (scaleFactor == 0.0f)
	{
		scaleFactor = 1.0f;
	}
	size /= scaleFactor;
}

const glm::mat4& gee::Drawable::getTransform()
{
	if (position != lastPosition_)
	{
		transform_.translate(position);
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
