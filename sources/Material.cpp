#include "..\headers\Material.h"
#include "../headers/vulkan_utils.h"

gee::Material::Material(const std::string& diffusePath, const std::string& normalPath, const std::string& specularPath) :
	diffuseTex{ diffusePath, gee::Texture::ColorSpace::NON_LINEAR}, normalTex{normalPath, gee::Texture::ColorSpace::LINEAR },
	specularTex{specularPath, gee::Texture::ColorSpace::LINEAR}
{
	
	gee::hash_combine(hash, diffuseTex.hash(), normalTex.hash(), specularTex.hash());
}