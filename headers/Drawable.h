#pragma once
#include <functional>
#include <string>
#include "Mesh.h"
#include "glm/glm.hpp"
#include "Units.h"
#include "Optics.h"
#include <optional>

namespace gee
{
	using namespace units::angle;
	struct Drawable
	{
		Drawable(const gee::Mesh& mesh, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const gee::Mesh& mesh, const Optics& optics, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const std::string& noun, const gee::Mesh& mesh, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0_deg });
		Drawable(const std::string& noun, const gee::Mesh& mesh, const Optics& optics, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& col = glm::vec3{ 1.0f }, const glm::vec3& rot = glm::vec3{ 0.0_deg });
	
		void setPosition(const glm::vec3& pos);
		void setColor(const glm::vec3& col);
		void setRotation(const glm::vec3& rot);
		bool hasLightComponent() const;
		const BoundingBox& boundingBox() const;
		gee::Optics& light();
		const size_t hash() const;
		glm::vec3 position;
		glm::vec3 size{ 1.0f };
		glm::vec4 color;
		glm::vec3 rotation;
		std::string name;
		float scaleFactor{ 1.0f };
		const gee::Mesh& mesh;
	private:
		std::optional<gee::Optics> light_;
		static uint32_t count; //solely used for default naming
		gee::BoundingBox boundingBox_;

		//normalize the scale factor so that the mesh bounding box volume is always 1m3
		const float normalizedScaleFactor(const gee::Mesh& mesh);
	};

}
