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

	struct ShaderMaterial
	{
		uint32_t diffuseIndex;
		uint32_t normalIndex;
		uint32_t specularIndex;
		uint32_t padding{};
	};

	template<class T>
	struct MaterialHelper
	{
		T diffuse;
		T normal;
		T specular;
	};

	template<class T>
	class MaterialHelperFactory
	{
	public:
		MaterialHelperFactory() = default;

		template<class TextureHolder>
		gee::MaterialHelper<T> create(TextureHolder& textureHolder, const gee::Material& texture);
	private:
		template<class TextureHolder>
		T addTexture(TextureHolder& textureHolder, const gee::Texture& texture);
	};

	template<class T>
	template<class TextureHolder>
	inline gee::MaterialHelper<T> MaterialHelperFactory<T>::create(TextureHolder& textureHolder, const gee::Material& material)
	{
		gee::MaterialHelper<T> helper;
		helper.diffuse = addTexture(textureHolder, material.diffuseTex);
		helper.normal = addTexture(textureHolder, material.normalTex);
		helper.specular = addTexture(textureHolder, material.specularTex);

		return std::move(helper);
	}
	template<class T>
	template<class TextureHolder>
	inline T MaterialHelperFactory<T>::addTexture(TextureHolder& textureHolder, const gee::Texture& texture)
	{
		textureHolder.get(texture.paths_[0], texture);
		return texture.paths_[0];
	}
}