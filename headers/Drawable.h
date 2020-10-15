#pragma once
#include <functional>
#include <string>
#include "Mesh.h"
#include "glm/glm.hpp"
#include "Units.h"

namespace gee
{
	using namespace units::angle;
	struct Drawable
	{
		Drawable(const gee::Mesh& mesh, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0_deg });
	
		void setPosition(const glm::vec3& pos);
		void setColor(const glm::vec3& col);
		void setRotation(const glm::vec3& rot);
		const size_t hash() const;
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 rotation;
		std::string name;
		const gee::Mesh& mesh;
	private:
		static uint32_t count; //solely used for default naming
	};

}
