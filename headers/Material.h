#pragma once
#include "Texture.h"

namespace gee
{
	struct Material
	{
		Material(const std::string& diffusePath = "../assets/default_textures/diffuse.png", const std::string& normalPath = "../assets/default_textures/normal.png", const std::string& specularPath = "../assets/default_textures/specular.png");
		Material(Material&&) = default;
		Material& operator=(Material&& other) = default;

		size_t hash{};
		Texture diffuseTex;
		Texture normalTex;
		Texture specularTex;
	};

	struct ShaderMaterial
	{
		uint32_t diffuseIndex;
		uint32_t normalIndex;
		uint32_t specularIndex;
		uint32_t padding{};
	};
	

	inline void hash_combine(std::size_t& seed) { }

	template <typename T, typename... Rest>
	inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) 
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}
}