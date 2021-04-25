#pragma once
#include "glm/ext/matrix_transform.hpp"

namespace gee
{
	class SpatialTransformation
	{
	public:
		const glm::mat4& translate(const glm::vec3& offset);
		const glm::mat4& rotate(const float offsetAngle, const glm::vec3& axis);
		const glm::mat4& scale(const glm::vec3& ratios);
		const glm::mat4& value() const;
	private:
		glm::mat4 matrix_{ 1.0f };
	};
}