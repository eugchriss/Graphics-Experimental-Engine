#pragma once
#include "Geometry.h"
#include "Material.h"
#include "MeshLoader.h"
#include "ResourceHolder.h"

namespace gee
{
	class StaticMesh
	{
	public:
		StaticMesh(gee::Geometry&& geoemtry, gee::Material&& material, const std::string& name = "no name");
		const Geometry& geometry() const;
		const Material& material() const;
	private:
		friend class MeshLoader;
		gee::Geometry geometry_;
		gee::Material material_;
		std::string name_{};
	};
	MAKE_REFERENCE(StaticMesh);
	template<>
	struct ResourceLoader<StaticMesh>
	{
		static StaticMesh load(const std::string& path)
		{
			gee::MeshLoader loader{};
			return loader.load(path);
		}
		static StaticMesh load(gee::ResourceHolder<gee::Texture>& textureHolder, const std::string& path)
		{
			gee::MeshLoader loader{textureHolder};
			return loader.load(path);
		}
	};
}
