#pragma once
#include <vector>
#include "ShaderEffect.h"
namespace gee
{
	struct FramebufferBuilder
	{
		uint32_t frameCount;
		std::vector<vkn::ShaderEffect> shaderEffects;
	};
}
