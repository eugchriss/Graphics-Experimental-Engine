#pragma once
#include <string>
#include <vector>

#include "Vertex.h"
#include "assimp/material.h"
#include "glm/glm.hpp"
#include "impl/vulkan/vulkan_utils.h"

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
	using GeometryRef = std::reference_wrapper<gee::Geometry>;
	using GeometryConstRef = std::reference_wrapper<const gee::Geometry>;

	class Mesh
	{
	public:
		Mesh(const std::string& name, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
		~Mesh() = default;
		Mesh(Mesh&&) = default;
		Geometry& geometry();
		const std::vector<gee::Vertex>& vertices() const;
		const std::vector<uint32_t>& indices() const;
		const std::string& name() const;
		const size_t hash() const;

	private:
		std::string name_;
		Geometry geometry_;
		uint32_t rendererID_{};
	};

	struct GeometryFactory
	{
		GeometryFactory() = default;
		Geometry& create(Geometry& geometry)
		{
			return geometry;
		}
	};
}

namespace std
{
	template<>
	struct hash<gee::GeometryConstRef>
	{
		size_t operator()(const gee::GeometryConstRef& geometry) const
		{
			return geometry.get().hash;
		}
	};

	template<>
	struct equal_to<gee::GeometryConstRef>
	{
		bool operator()(const gee::GeometryConstRef& lhs, const gee::GeometryConstRef& rhs) const
		{
			return lhs.get().hash == rhs.get().hash;
		}
	};
}