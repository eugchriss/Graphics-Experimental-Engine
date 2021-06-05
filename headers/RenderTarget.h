#pragma once
#include <glm/glm.hpp>
#include "enums.h"

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
}
