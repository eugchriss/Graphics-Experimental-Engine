#pragma once
#include "Material.h"
#include "Mesh.h"

namespace gee
{
	class MaterialInstance
	{
	public:
		MaterialInstance(vkn::Material& material) : materialRef_{ std::ref(material) }, hash{ material.hash() }
		{
		}
		MaterialInstance(MaterialInstance&& other) = default;
		void set_property(const TEXTURE_SLOT slot, const gee::Texture& texture)
		{
			textureSlots.emplace(slot, std::ref(texture));
			gee::hash_combine(hash, std::hash<std::string>{}(texture.name()));
		}
		void add_geometry(const gee::Geometry& geometry, const glm::mat4& transform)
		{
			geometries[std::ref(geometry)].emplace_back(transform);
		}
		void reset_transforms()
		{
			for (auto& [geometryRef, transforms] : geometries)
			{
				transforms.clear();
			}
		}
		std::unordered_map<gee::GeometryConstRef, std::vector<glm::mat4>> geometries;
		std::unordered_map<TEXTURE_SLOT, gee::TextureConstRef> textureSlots;
		vkn::MaterialRef materialRef_;
		size_t hash{};
	};
	MAKE_REFERENCE(MaterialInstance)
}
