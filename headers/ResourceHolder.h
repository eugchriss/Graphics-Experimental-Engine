#pragma once
#include <string>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace gee
{
	template<class T>
	struct ResourceLoader
	{
		template<class ... Args>
		static T load(Args&&... args)
		{
			return { std::forward<Args>(args)... };
		}
	};

	template<class Resource, class Key = std::string>
	class ResourceHolder
	{
	public:
		ResourceHolder() = default;
		ResourceHolder(ResourceHolder&&) = default;
		Resource& get(const Key& key);
		template<class ... Args>
		Resource& get(const Key& key, Args&&... args);

	private:
		using Container_t = std::unordered_map<Key, Resource>;
		Container_t resources_;
	};
}

template<class Resource, class Key>
inline Resource& gee::ResourceHolder<Resource, Key>::get(const Key& key)
{
	auto result = resources_.find(key);
	if (result != std::end(resources_))
	{
		return result->second;
	}
	else
	{
		if constexpr (std::is_default_constructible<Resource>::value)
		{
			auto resource = resources_.emplace(key, Resource{});
			return resource.first->second;
		}
		else
		{
			throw std::runtime_error{ "The specified element doesn t exist yet and can t be default constructed" };
		}
	}
}
template<class Resource, class Key>
template< class ...Args>
inline Resource& gee::ResourceHolder<Resource, Key>::get(const Key& key, Args&&... args)
{
	auto result = resources_.find(key);
	if (result == std::end(resources_))
	{
		auto resource = resources_.emplace(key, std::move(gee::ResourceLoader<Resource>::load(std::forward<Args>(args)...)));
		return resource.first->second;
	}
	else
	{
		return result->second;
	}
}
