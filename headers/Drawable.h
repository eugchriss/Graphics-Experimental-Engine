#pragma once
#include <functional>
#include <optional>
#include <string>

#include "MaterialInstance.h"
#include "Mesh.h"
#include "Optics.h"
#include "SpatialTransformation.h"
#include "Units.h"

namespace gee
{
	using namespace units::angle;
	struct Drawable
	{
		Drawable(const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const std::string& name, const gee::Geometry& geometry, gee::MaterialInstance& materialInstance, const glm::vec3& pos = glm::vec3{0.0f}, const glm::vec3& rot = glm::vec3{ 0.0_deg });
		Drawable(Drawable&&) = default;
		const glm::mat4& getTransform();
		std::string name;
		float scaleFactor{ 1.0f };
		gee::GeometryConstRef geometry;
		gee::MaterialInstanceRef materialInstance;
		glm::vec3 position{};
		glm::vec3 size{ 1.0f };
		glm::vec3 rotation{};

	private:
		SpatialTransformation transform_;
		glm::vec3 lastPosition_{};
		glm::vec3 lastSize_{ 1.0f };
		glm::vec3 lastRotation_{};
		static uint32_t count; //solely used for default naming

		//normalize the scale factor so that the mesh bounding box volume is always 1m3
		const float normalizedScaleFactor(const gee::Mesh& mesh);
	};
	MAKE_REFERENCE(Drawable)
}