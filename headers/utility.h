#pragma once
#include <queue>
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

    template<class T>
    struct Next
    {
        static T get(const T& x)
        {
            return x + 1;
        }
    };

    template<class T>
    class IdDispenser
    {
    public:
        IdDispenser() = default;
        IdDispenser(IdDispenser&&) = default;

        const T get()
        {
            if (!std::empty(recyclableIDs_))
            {
                auto id = recyclableIDs_.front();
                recyclableIDs_.pop();
                return id;
            }
            else
            {
                auto used = id;
                inUseIDs_.emplace_back(used);
                id = Next<T>::get(id);
                return used;
            }
        }

        void consumed(T& id)
        {
            recyclableIDs_.push(id);
            auto result = std::find(std::begin(inUseIDs_), std::end(inUseIDs_), id);
            if (result == std::end(inUseIDs_))
            {
                throw std::runtime_error{ "Trying to delete a non reported id" };
            }
            else
            {
				inUseIDs_.erase(result);
            }
        }
    private:
        T id{};
        std::vector<T> inUseIDs_;
        std::queue<T> recyclableIDs_;
    };

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

