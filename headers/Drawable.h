#pragma once
#include <functional>
#include <string>
#include "Mesh.h"
#include "glm/glm.hpp"
namespace gee
{
	struct Drawable
	{
		Drawable(const gee::Mesh& mesh, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& col = glm::vec3{ 1.0f });
		Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos, const glm::vec3& col);
	
		void setPosition(const glm::vec3& pos);
		void setColor(const glm::vec3& col);
		const size_t hash() const;
		glm::vec3 position;
		glm::vec4 color;
		std::string name;
		const gee::Mesh& mesh;
	private:
		static uint32_t count; //solely used for default naming
	};

}
