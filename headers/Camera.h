#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "Units.h"

#include <utility>
#include <string>

namespace gee
{
	using namespace units::angle;
	using namespace units::length;

	enum DIRECTION
	{
		FORWARD,
		BACKWARD,
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	class Plane
	{
	public:
		Plane() = default;
		Plane(const glm::vec3& normal, const glm::vec3 point);
		inline float distanceToPoint(const glm::vec3& point) const
		{
			return normal.x * point.x + normal.y * point.y + normal.z * point.z + d;
		}
		Plane& operator = (const Plane& other);
	private:
		glm::vec3 normal{};
		float d{};
	};

	struct ViewFrustum
	{
		Plane nearPlane;
		Plane farPlane;
		Plane leftPlane;
		Plane rightPlane;
		Plane topPlane;
		Plane bottomPlane;

		bool isInside(const glm::vec3& pos) const;
	};

	struct Sensibility
	{
		float right = 1.0_m;
		float left = 1.0_m;
		float up = 1.0_m;
		float down = 1.0_m;
		float forward = 100.0_m;
		float backward = 100.0_m;

		float pitch = 10.0_deg;
		float yaw = 10.0_deg;
		float roll = 10.0_deg;
	};

	class Camera
	{
	public:
		Camera(const glm::vec3 target = glm::vec3{ 0.0f, 0.0f, 0.0f }, const glm::vec3 pos = glm::vec3{ 0.0f, 0.0f, 1.0_km }, const glm::vec3 woldUp = glm::vec3(0.0f, 1.0f, 0.0f));
		~Camera();
		void move(const DIRECTION d);
		void rotate(const double pitch, const double yaw);
		void arcball(const double pitch, const double yaw);
		const glm::mat4 pointOfView() const;
		const glm::mat4 perspectiveProjection(const float aspectRatio) const;
		inline bool isViewable(const glm::vec3& pos) const
		{
			return viewFrustum_.isInside(pos);
		}
		void imguiDisplay();
		glm::vec3 position_;
		glm::vec3 front_;

		Sensibility sensibility{};
		std::string label;

	private:
		glm::vec3 target_;

		glm::vec3 worldUp_;
		glm::vec3 up_;
		glm::vec3 right_;

		float fov_ = 45.0_deg;
		float viewRange_ = 5.0_km;
		float pitch_{ 180.0_deg };
		float yaw_ = 90.0_deg;
		float absolutePitch_{};
		float absouluteYaw_{};
		bool moved_{ false };

		ViewFrustum viewFrustum_;
		void createCameraSpace();
		const ViewFrustum getViewFrustum();


		//Gui elements
		float speed{ 100.0_m };
		bool synchronizeSensibility_{ true };

		void synchronizeSensibility(const float value);
		//only used to give a different	default label to each new instance
		static unsigned	int count_;
	};
}