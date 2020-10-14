#include "..\headers\Drawable.h"

uint32_t gee::Drawable::count{};

gee::Drawable::Drawable(const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col):
	mesh{mesh},
	position{pos}, color{col, 1.0f}
{
	name = std::string{ "Drawable : " } + std::to_string(count);
	count++;
}

gee::Drawable::Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col) :
	name{ noun }, mesh{ mesh },
	position{ pos }, color{ col, 1.0f }
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

const size_t gee::Drawable::hash() const
{
	return std::hash<std::string>{}(name);
}
