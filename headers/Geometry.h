#pragma once
#include <vector>

#include "utility.h"
#include "Vertex.h"

namespace gee
{
	struct Geometry
	{
		Geometry(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
		Geometry(const Geometry&) = delete;
		Geometry& operator = (const Geometry&) = delete;
		Geometry(Geometry&&) = default;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		size_t hash{};
	};
	MAKE_CONST_REFERENCE(Geometry);
	gee::Geometry getCubeGeometry();
	gee::Geometry getQuadGeometry();
	gee::Geometry getFloorGeometry();

	template<class T> struct ID;
	template<>
	struct ID<GeometryConstRef>
	{
		using Type = size_t;
		static Type get(GeometryConstRef g)
		{
			return Type{};
		}
	};
}