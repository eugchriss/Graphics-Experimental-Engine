#include "..\headers\Drawable.h"

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot):
	mesh{mesh},
	position{pos}, color{col, 1.0f}, rotation{rot}
{
	name = std::string{ "Drawable : " } + std::to_string(count);
	count++;
}

gee::Drawable::Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col, const glm::vec3& rot ) :
	name{ noun }, mesh{ mesh },
	position{ pos }, color{ col, 1.0f }, rotation{rot}
{
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
