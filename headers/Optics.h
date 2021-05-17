#pragma once
#include <variant>

#include "glm/glm.hpp"

namespace gee
{
	struct PointLight
	{
		PointLight(const glm::vec3& pos = {});
		PointLight(const PointLight&) = default;
		
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
		float padding1;
		float padding2;
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