#pragma once
#include <vector>

#include "Pass.h"
namespace gee
{
	namespace vkn { class Renderpass; }
	class Renderpass
	{
	public:
		Renderpass() = default;
		void add_pass(const Pass& pass);
	private:
		friend ID<Renderpass>;
		friend struct ResourceLoader<vkn::Renderpass>;
		std::vector<Pass> passes_;
	};

	template<>
	struct ID<Renderpass>
	{
		using Type = size_t;
		static Type get(const Renderpass& rp)
		{
			Type seed{};
			for (auto& pass : rp.passes_)
			{
				gee::hash_combine(seed, ID<Pass>::get(pass));
			}
			return seed;
		}
	};
}
