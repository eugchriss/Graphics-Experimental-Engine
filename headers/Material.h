#pragma once
#include "Texture.h"

namespace gee
{
	struct Material
	{
		Material(const std::string& diffusePath = "../assets/default_textures/diffuse.png", const std::string& normalPath = "../assets/default_textures/normal.png", const std::string& specularPath = "../assets/default_textures/specular.png");
		Material(const std::array<std::string, 6>& diffusePaths , const std::string& normalPath = "../assets/default_textures/normal.png", const std::string& specularPath = "../assets/default_textures/specular.png");
		Material(Material&&) = default;
		Material& operator=(Material&& other) = default;

		size_t hash{};
		Texture diffuseTex;
		Texture normalTex;
		Texture specularTex;
	};
}