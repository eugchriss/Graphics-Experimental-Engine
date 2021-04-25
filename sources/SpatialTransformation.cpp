#include "..\headers\SpatialTransformation.h"

const glm::mat4& gee::SpatialTransformation::translate(const glm::vec3& offset)
{
	matrix_ = glm::translate(matrix_, offset);
	return matrix_;
}

const glm::mat4& gee::SpatialTransformation::rotate(const float offsetAngle, const glm::vec3& axis)
{
	matrix_ = glm::rotate(matrix_, offsetAngle, axis);
	return matrix_;
}

const glm::mat4& gee::SpatialTransformation::scale(const glm::vec3& ratios)
{
	matrix_ = glm::scale(matrix_, ratios);
	return matrix_;
}

const glm::mat4& gee::SpatialTransformation::value() const
{
	return matrix_;
}
