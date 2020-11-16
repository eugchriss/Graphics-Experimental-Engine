#define GLM_ENABLE_EXPERIMENTAL
#include "../headers/Camera.h"
#include "glm/gtx/rotate_vector.hpp"
#include <iostream>
#include "../headers/imgui_impl_glfw.h"

void gee::Camera::createCameraSpace()
{
	front_ = glm::normalize(target_ - position_);
	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));
}

gee::Camera::Camera(const glm::vec3 target, const glm::vec3 pos, const glm::vec3 worldUpDirection) : target_{ target }, position_{ pos }, worldUp_{ worldUpDirection }
{
	createCameraSpace();
	viewFrustum_ = getViewFrustum();
}

void gee::Camera::move(const DIRECTION direction)
{
	switch (direction)
	{
	case FORWARD:
		position_ += sensibility.forward * front_;
		break;
	case BACKWARD:
		position_ -= sensibility.backward * front_;
		break;
	case LEFT:
		position_ -= sensibility.left * right_;
		break;
	case RIGHT:
		position_ += sensibility.right * right_;
		break;
	}
	viewFrustum_ = getViewFrustum();
}

void gee::Camera::rotate(const double pitch, const double yaw)
{
	pitch_ -= glm::radians(pitch) * sensibility.pitch;
	yaw_ += glm::radians(yaw) * sensibility.yaw;

	front_.x = std::cos(pitch_) * std::cos(yaw_);
	front_.y = std::sin(pitch_);
	front_.z = std::cos(pitch_) * std::sin(yaw_);

	front_ = glm::normalize(front_);

	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));
	viewFrustum_ = getViewFrustum();
}

void gee::Camera::arcball(const double pitch, const double yaw)
{
	position_.x = std::cos(pitch_) * std::cos(yaw_);
	position_.y = std::sin((pitch_));
	position_.z = std::cos(pitch_) * std::sin(yaw_);

	right_ = glm::normalize(glm::cross(front_, worldUp_));
	up_ = glm::normalize(glm::cross(right_, front_));
	viewFrustum_ = getViewFrustum();
}

const glm::mat4 gee::Camera::pointOfView() const
{
	return glm::lookAt(position_, position_ + front_, worldUp_);
}

const glm::mat4 gee::Camera::perspectiveProjection(const float aspectRatio) const
{
	
	return glm::perspective(fov_, aspectRatio, 0.1f, viewRange_);
}

const glm::mat4 gee::Camera::orhtogonalProjection() const
{
	return glm::ortho(-viewRange_, viewRange_, -viewRange_, viewRange_, 0.1f, viewRange_);
}

void gee::Camera::imguiDisplay()
{
	bool open{ true };
	ImGui::Begin("Camera Settings", &open, 0);
	ImGui::InputFloat("Maximum speed", &speed, 0.0f, viewRange_, "%.1f meters/step"); ImGui::SameLine();
	ImGui::Checkbox("Synchronize ", &synchronizeSensibility_);
	if (synchronizeSensibility_)
	{
		synchronizeSensibility(speed);
	}
	ImGui::SliderFloat("Forward sensibility", &sensibility.forward, 0.0f, speed, " %.1f meters/step");
	ImGui::SliderFloat("Backward sensibility", &sensibility.backward, 0.0f, speed, " %.1f meters/step");
	ImGui::End();

}

const gee::ViewFrustum gee::Camera::getViewFrustum()
{
	auto nearCenter = front_ * 0.1f + position_;
	auto farCenter = position_ + front_ * viewRange_;
	auto nearSide = std::tan(fov_ / 2.0f) * viewRange_;

	ViewFrustum viewFrustum{};
	viewFrustum.nearPlane = ViewFrustum::Plane(front_, nearCenter);
	viewFrustum.farPlane = ViewFrustum::Plane(-front_, farCenter);
	viewFrustum.leftPlane = ViewFrustum::Plane(glm::rotate(right_, fov_ / 2.0f, up_), farCenter - right_ * nearSide);
	viewFrustum.rightPlane = ViewFrustum::Plane(-glm::rotate(right_, fov_ / 2.0f, -up_), farCenter + right_ * nearSide);
	viewFrustum.topPlane = ViewFrustum::Plane(-glm::rotate(up_, fov_ / 2.0f, right_), farCenter + up_ * nearSide);
	viewFrustum.bottomPlane = ViewFrustum::Plane(glm::rotate(up_, fov_ / 2.0f, -right_), farCenter - up_ * nearSide);
	return viewFrustum;
}

void gee::Camera::synchronizeSensibility(const float value)
{
	sensibility.backward = value;
	sensibility.forward = value;
}

gee::Camera::~Camera()
{
}

bool gee::ViewFrustum::isInside(const glm::vec3& pos) const
{
	float radius = 0.5f;
	auto pPos = pos + radius;
	auto nPos = pos - radius;
	if (nearPlane.distanceToPoint(pPos) < 0.0f || farPlane.distanceToPoint(pPos) < 0.0f)
	{
		return false;
	}
	else
	{
		if (leftPlane.distanceToPoint(pPos) < 0.0f || rightPlane.distanceToPoint(nPos) < 0.0f)
		{
			return false;
		}
		else
		{
			if (topPlane.distanceToPoint(pPos) < 0.0f || bottomPlane.distanceToPoint(nPos) < 0.0f)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}

}

gee::ViewFrustum::Plane::Plane(const glm::vec3& norm, const glm::vec3 point) : normal(norm)
{
	d = -normal.x * point.x - normal.y * point.y - normal.z * point.z;
}

gee::ViewFrustum::Plane& gee::ViewFrustum::Plane::operator=(const Plane& other)
{
	normal = other.normal;
	d = other.d;
	return *this;
}