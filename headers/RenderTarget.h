#pragma once
#include <glm/glm.hpp>
#include "enums.h"
#include "utility.h"

namespace gee
{
	struct RenderTarget
	{
		glm::u32vec2 size{};
		gee::TargetFormat format{};
		gee::TargetUsage usage{};
		gee::TargetOp loadOp{TargetOp::DONT_CARE};
		gee::TargetOp storeOp{TargetOp::DONT_CARE};
	};

	template<class T> struct ID;
	template<>
	struct ID<RenderTarget>
	{
		using Type = size_t;
		static Type get(const RenderTarget& target)
		{
			Type seed{};
			gee::hash_combine(seed, target.size.x, target.size.y, static_cast<size_t>(target.usage), static_cast<size_t>(target.loadOp), static_cast<size_t>(target.storeOp));
			return seed;
		}
	};
}
