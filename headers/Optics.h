#pragma once
#include "glm/glm.hpp"
#include <variant>

namespace gee
{
	enum class Light
	{
		PointLight,
		SpotLight,
		DirectionalLight
	};

	struct Optics
	{
		Optics(const Light type_, const glm::vec3& pos = {});
		Optics(const Optics&) = default;
		Light type;
		
		glm::vec3 position{};
		glm::vec3 direction{ 0.0f, 0.f, 1.0f };
		glm::vec3 ambient{ 0.2f };
		glm::vec3 diffuse{ 1.0f };
		glm::vec3 specular{};
		float linear{};
		float quadratic{};
		float cutOff{ 1.0f };
	};

	struct ShaderPointLight
	{
		glm::vec4 position{};
		glm::vec4 ambient{ 0.2f };
		glm::vec4 diffuse{ 1.0f };
		glm::vec4 specular{};
		float linear{};
		float quadratic{};
	};

	struct ShaderSpotLight
	{
		glm::vec4 position{};
		glm::vec4 direction{ 0.0f, 0.0f, 1.0f, 1.0f };
		glm::vec4 ambient{ 0.2f };
		glm::vec4 diffuse{ 1.0f };
		glm::vec4 specular{};
		float cutOff{ 1.0f };
		float padding{};
	};

	struct ShaderDirectionalLight
	{
		glm::vec4 direction{ 0.0f, 0.f, 1.0f, 1.0f };
		glm::vec4 ambient{ 0.2f };
		glm::vec4 diffuse{ 1.0f };
		glm::vec4 specular{};
	};
}