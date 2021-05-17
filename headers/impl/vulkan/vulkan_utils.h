#pragma once
#include <string>

#include "spirv_cross/spirv_glsl.hpp"
#include "vulkan/vulkan.hpp"

namespace
{
	template<class T>
	struct is_array : std::false_type {};

	template <class T>
	struct is_array<std::vector<T>> : std::true_type {};
}

namespace vkn
{
	void error_check(const VkResult result, const std::string& msg);
	struct Format
	{
		VkFormat format{ VK_FORMAT_UNDEFINED };
		uint32_t range{};
	};
	const Format getFormat(const spirv_cross::SPIRType& type);
	template<typename T>
	class Observer_ptr
	{
	public:
		Observer_ptr() = default;
		Observer_ptr(T& resource) : ptr_{ &resource } {}
		Observer_ptr& operator= (const Observer_ptr<T>& other)
		{
			ptr_ = other.ptr_;
			return *this;
		}
		T* operator->()
		{
			if (!ptr_)
			{
				throw std::runtime_error{ "Nullptr dereferencing" };
			}
			return ptr_;
		}
		T* operator->() const
		{
			if (!ptr_)
			{
				throw std::runtime_error{ "Nullptr dereferencing" };
			}
			return ptr_;
		}
	private:
		T* ptr_;
	};


	

}
namespace gee
{
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
			seed ^= std::hash<T>{}(element) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
}