#pragma once
#include <functional>
#include <string>
#include "Mesh.h"
#include "glm/glm.hpp"
#include "Units.h"
#include "Optics.h"
#include "BoundingBox.h"
#include <optional>
#include "MaterialInstance.h"

namespace gee
{
	using namespace units::angle;
	struct Drawable
	{
		Drawable(const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const std::string& name, const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& rot = glm::vec3{ 0.0_deg });
		Drawable(Drawable&&) = default;
		void setPosition(const glm::vec3& pos);
		void setRotation(const glm::vec3& rot);
		void setSize(const glm::vec3& size);
		const glm::vec3& getPosition() const;
		const glm::vec3& getSize() const;
		const glm::vec3& getRotation() const;
		const glm::mat4& getTransform();
		const glm::mat4& getNormalMatrix() const;
		const size_t hash() const;
		std::string name;
		float scaleFactor{ 1.0f };
		gee::GeometryConstRef geometry;
		gee::MaterialInstanceRef materialInstance;
	private:
		glm::vec3 position_{};
		glm::vec3 size_{ 1.0f };
		glm::vec3 rotation_{};
		glm::mat4 transform_{ 1.0f };
		glm::mat4 normalMatrix_{ 1.0f };
		bool shouldRecomputeTransform_ = false;
		static uint32_t count; //solely used for default naming

		//normalize the scale factor so that the mesh bounding box volume is always 1m3
		const float normalizedScaleFactor(const gee::Mesh& mesh);
	};
	MAKE_REFERENCE(Drawable)
}
