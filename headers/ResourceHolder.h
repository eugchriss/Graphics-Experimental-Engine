#pragma once
#include <string>
#include <type_traits>
#include <unordered_map>

namespace gee
{
	template<class Factory, class Resource, class Key = std::string>
	class ResourceHolder
	{
		using Container_t = std::unordered_map<Key, Resource>;
	public:
		ResourceHolder(Factory& f);
		~ResourceHolder() = default;

		Resource& get(const Key& key);
		template<class ... Args>
		Resource& get(const Key& key, Args& ...args);

	private:
		Container_t resources_;
		Factory factory_;
	};
}

template<class Factory, class Resource, class Key>
inline gee::ResourceHolder<Factory, Resource, Key>::ResourceHolder(Factory& f) : factory_{ std::move(f) }
{
}

template<class Factory, class Resource, class Key>
inline Resource& gee::ResourceHolder<Factory, Resource, Key>::get(const Key& key)
{
	auto& result = resources_.find(key);
	if (result != std::end(resources_))
	{
		return result->second;
	}
	else
	{
		if constexpr (std::is_default_constructible<Resource>::value)
		{
			auto& resource = resources_.emplace(key, Resource{});
			return resource.first->second;
		}
		else
		{
			throw std::runtime_error{ "The specified element doesn t exist yet and can t be default constructed" };
		}
	}	
}

template<class Factory, class Resource, class Key>
template< class ...Args>
inline Resource& gee::ResourceHolder<Factory, Resource, Key>::get(const Key& key, Args& ...args)
{
	auto& result = resources_.find(key);
	if (result == std::end(resources_))
	{
		if constexpr (std::is_copy_constructible<Resource>::value)
		{
			auto& resource = resources_.emplace(key, factory_.create(std::forward<Args>(args)...));
			return resource.first->second;
		}
		else
		{
			auto& resource = resources_.emplace(key, std::move(factory_.create(std::forward<Args>(args)...)));
			return resource.first->second;
		}
	}
	else
	{
		return result->second;
	}
}
