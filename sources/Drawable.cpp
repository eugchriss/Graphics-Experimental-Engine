#include "..\headers\Drawable.h"

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot):
	mesh{mesh},
	position{pos}, color{col, 1.0f}, rotation{rot}
{
	name = std::string{ "Drawable : " } + std::to_string(count);
	scaleFactor = normalizedScaleFactor(mesh);
	size /= scaleFactor;
	count++;
}

gee::Drawable::Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot ) :
	name{ noun }, mesh{ mesh },
	position{ pos }, color{ col, 1.0f }, rotation{rot}
{
	scaleFactor = normalizedScaleFactor(mesh);
	size /= scaleFactor;
}	

void gee::Drawable::setPosition(const glm::vec3& pos)
{
	position = pos;
}

void gee::Drawable::setColor(const glm::vec3& col)
{
	color = glm::vec4{ col, 1.0f };
}

void gee::Drawable::setRotation(const glm::vec3& rot)
{
	rotation = rot;
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
	size = glm::vec3{ x, y, z };
	return glm::pow(volume, 1 / 3.0f);
}
