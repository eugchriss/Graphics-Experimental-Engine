#pragma once
#include <type_traits>
#include <string>
#include <unordered_map>

namespace gee
{
	template<class Hashable>
	class Occurence
	{
		using Container_t = std::unordered_map<Hashable, uint64_t>;
	public:
		Occurence();
		void increase(const Hashable& hashable, uint64_t amount = 1);
		void decrease(const Hashable& hashable, uint64_t amount = 1);
		const uint64_t get(const Hashable&) const;
		void reset();
		typename Container_t::iterator begin();
		typename Container_t::const_iterator begin() const;
		typename Container_t::iterator end();
		typename Container_t::const_iterator end() const;

	private:
		Container_t occurence_;
	};

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

		const size_t indexOf(const Key& key) const;
		uint64_t occurence(const Key& key) const;
		const Occurence<Key>& occurences() const;
		void resetOccurences();
		typename Container_t::iterator begin();
		typename Container_t::const_iterator begin() const;
		typename Container_t::iterator end();
		typename Container_t::const_iterator end() const;
		const size_t size() const;
	private:
		Container_t resources_;
		Occurence<Key> occurences_;
		Factory factory_;
	};

	template<class Hashable>
	inline Occurence<Hashable>::Occurence()
	{
	}
	template<class Hashable>
	inline void Occurence<Hashable>::increase(const Hashable& hashable, uint64_t amount)
	{
		auto result = occurence_.find(hashable);
		if (result == std::end(occurence_))
		{
			occurence_[hashable] = amount;
		}
		else
		{
			result->second += amount;
			assert(result->second > amount && "Reached the maximum countable number");
		}
	}
	template<class Hashable>
	inline void Occurence<Hashable>::decrease(const Hashable& hashable, uint64_t amount)
	{
		auto result = occurence_.find(hashable);
		if (result == std::end(occurence_))
		{
			occurence_[hashable] = 0;
		}
		else
		{
			if (amount > result->second)
			{
				result->second = 0; 
			}
			else
			{
				result->second -= amount;
			}
		}
	}
	template<class Hashable>
	inline const uint64_t Occurence<Hashable>::get(const Hashable& hashable) const
	{
		auto result = occurence_.find(hashable);
		if (result == std::end(occurence_))
		{
			return 0;
		}
		else
		{
			return result->second;
		}
	}
	template<class Hashable>
	inline void Occurence<Hashable>::reset()
	{
		occurence_.clear();
	}
	template<class Hashable>
	inline typename Occurence<Hashable>::Container_t::iterator Occurence<Hashable>::begin()
	{
		return occurence_.begin();
	}
	template<class Hashable>
	inline typename Occurence<Hashable>::Container_t::iterator Occurence<Hashable>::end()
	{
		return occurence_.end();
	}
	template<class Hashable>
	inline typename Occurence<Hashable>::Container_t::const_iterator Occurence<Hashable>::begin() const
	{
		return occurence_.begin();
	}
	template<class Hashable>
	inline typename Occurence<Hashable>::Container_t::const_iterator Occurence<Hashable>::end() const
	{
		return occurence_.end();
	}
}

template<class Factory, class Resource, class Key>
inline gee::ResourceHolder<Factory, Resource, Key>::ResourceHolder(Factory& f) : factory_{ std::move(f) }
{
}

template<class Factory, class Resource, class Key>
inline Resource& gee::ResourceHolder<Factory, Resource, Key>::get(const Key& key)
{
	auto result = resources_.find(key);
	if (result != std::end(resources_))
	{
		occurences_.increase(key);
		return result->second;
	}
	else
	{
		if constexpr (std::is_default_constructible<Resource>::value)
		{
			occurences_.increase(key);
			auto resource = resources_.emplace(key, Resource{});
			return resource.first->second;
		}
		else
		{
			throw std::runtime_error{ "The specified element doesn t exist yet and can t be default constructed" };
		}
	}	
}

template<class Factory, class Resource, class Key>
inline const size_t gee::ResourceHolder<Factory, Resource, Key>::indexOf(const Key& key) const
{
	auto result = resources_.find(key);
	assert(result != resources_.end() && "Indexing a non existent element");

	return std::distance(resources_.begin(), result);
}

template<class Factory, class Resource, class Key>
inline uint64_t gee::ResourceHolder<Factory, Resource, Key>::occurence(const Key& key) const
{
	return occurences_.get(key);
}

template<class Factory, class Resource, class Key>
inline const gee::Occurence<Key>& gee::ResourceHolder<Factory, Resource, Key>::occurences() const
{
	return occurences_;
}

template<class Factory, class Resource, class Key>
inline void gee::ResourceHolder<Factory, Resource, Key>::resetOccurences()
{
	occurences_.reset();
}

template<class Factory, class Resource, class Key>
inline typename gee::ResourceHolder<Factory, Resource, Key>::Container_t::iterator gee::ResourceHolder<Factory, Resource, Key>::begin()
{
	return resources_.begin();
}

template<class Factory, class Resource, class Key>
inline typename gee::ResourceHolder<Factory, Resource, Key>::Container_t::const_iterator gee::ResourceHolder<Factory, Resource, Key>::begin() const
{
	return resources_.begin();
}

template<class Factory, class Resource, class Key>
inline typename gee::ResourceHolder<Factory, Resource, Key>::Container_t::iterator gee::ResourceHolder<Factory, Resource, Key>::end()
{
	return resources_.end();
}

template<class Factory, class Resource, class Key>
inline typename gee::ResourceHolder<Factory, Resource, Key>::Container_t::const_iterator gee::ResourceHolder<Factory, Resource, Key>::end() const
{
	return resources_.end();
}

template<class Factory, class Resource, class Key>
inline const size_t gee::ResourceHolder<Factory, Resource, Key>::size() const
{
	return resources_.size();
}

template<class Factory, class Resource, class Key>
template< class ...Args>
inline Resource& gee::ResourceHolder<Factory, Resource, Key>::get(const Key& key, Args& ...args)
{
	occurences_.increase(key);
	auto result = resources_.find(key);
	if (result == std::end(resources_))
	{
		auto resource = resources_.emplace(key, factory_.create(std::forward<Args>(args)...));
		return resource.first->second;

	}
	else
	{
		return result->second;
	}
}
