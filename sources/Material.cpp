#include "..\headers\Material.h"

gee::Material::Material(const std::string& diffusePath, const std::string& normalPath, const std::string& specularPath) :
	diffuseTex{ diffusePath }, normalTex{normalPath},
	specularTex{specularPath}
{
	gee::hash_combine(hash, diffuseTex.hash(), normalTex.hash(), specularTex.hash());
}