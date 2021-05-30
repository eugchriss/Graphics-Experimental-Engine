#pragma once
#include <vector>

#include "Mesh.h"
#include "glm/glm.hpp"

namespace gee
{
#define MAKE_REFERENCE(object) using object##Ref = std::reference_wrapper<object>;
#define MAKE_CONST_REFERENCE(object) using object##ConstRef = std::reference_wrapper<const object>;
#define MAKE_UNIQUE_PTR(object) using object##Ptr = std::unique_ptr<object>;

	template<class T>
	struct ID {};
	inline void hash_combine(std::size_t& seed) { }
	template <typename T, typename... Rest>
	inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		hash_combine(seed, rest...);
	}

	template<class Container>
	inline const std::size_t hash_combine(const Container& c)
	{
		using T = typename Container::value_type;
		std::size_t seed{};
		for (const auto& element : c)
		{
			seed ^= std::hash<T>{}(element)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	void processTangentSpace(std::vector<gee::Vertex>& vertices, std::vector<uint32_t>& indices);
}

