#pragma once
#include "../headers/ModelLoader.h"

#include <type_traits>
#include <string>
#include <unordered_map>

template<class Resource, class Key = std::string>
class ResourceHolder
{
public:
	ResourceHolder() = default;
	~ResourceHolder() = default;

	Resource& get(const Key& key);
	Resource& get(const Key& key, const std::string& meshPath);

private:
	std::unordered_map<Key, Resource> resources_;
};

template<class Resource, class Key>
inline Resource& ResourceHolder<Resource, Key>::get(const Key& key)
{
	if constexpr (std::is_default_constructible<Resource>::value)
	{
		auto resource = resources_.try_emplace(key);
		return *resource.first;
	}
	else
	{
		throw std::runtime_error{ "The specified element doesn t exist yet and can t be default constructed" };
	}
}

template<class Resource, class Key>
inline Resource& ResourceHolder<Resource, Key>::get(const Key& key, const std::string& path)
{
	gee::ModelLoader loader{};
	auto resource = resources_.try_emplace(key, loader.load(path));
	return resource.first->second;
}