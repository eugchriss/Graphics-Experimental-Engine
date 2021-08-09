#pragma once
#include <functional>
#include <optional>
#include <string>

#include "Material.h"
#include "Optics.h"
#include "SpatialTransformation.h"
#include "StaticMesh.h"
#include "Units.h"
#include "utility.h"

namespace gee
{
	class Drawable
	{
	public:
		Drawable(gee::StaticMesh& m, const glm::vec3& pos = glm::vec3{ 0.0f }, const glm::vec3& rot = glm::vec3{ 0.0f });
		Drawable(const Drawable&) = delete;
		Drawable(Drawable&&);
		~Drawable();
		const gee::Material& material() const;
		const gee::Geometry& geometry() const;
		const glm::mat4& getTransform();
		float scaleFactor{ 1.0f };
		glm::vec3 position{};
		glm::vec3 size{ 1.0f };
		glm::vec3 rotation{};
		bool operator==(const Drawable& other) const;
	private:
		gee::StaticMeshRef mesh;
		SpatialTransformation transform_;
		glm::vec3 lastPosition_{};
		glm::vec3 lastSize_{ 1.0f };
		glm::vec3 lastRotation_{};
		static IdDispenser<size_t> idDispenser_;
		friend ID<Drawable>;
		size_t id{};
		bool shouldDeleteId_{ true };
		//normalize the scale factor so that the mesh bounding box volume is always 1m3
		//const float normalizedScaleFactor(const gee::Mesh& mesh);
	};
	MAKE_REFERENCE(Drawable);
	MAKE_CONST_REFERENCE(Drawable);
}