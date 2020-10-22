#pragma once
#include <utility>
#include "glm/glm.hpp"
#include "Drawable.h"

namespace gee
{
	struct PointLight : public Drawable
	{
		template<class... Args>
		PointLight(Args&&...  args);
		glm::vec3 ambient{ 0.2f };
		glm::vec3 diffuse{ 1.0f };
		glm::vec3 specular{};
		float linear{};
		float quadratic{};
	};

 	struct ShaderPointLight
	{
		glm::vec4 position;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float linear;
		float quadratic;
	};

	template<class ...Args>
	inline PointLight::PointLight(Args&& ...args): Drawable{std::forward<Args>(args)...}
	{
	}
}