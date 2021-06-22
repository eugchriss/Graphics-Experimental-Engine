#include "../headers/Pass.h"

gee::Pass::Pass(const ShaderTechnique& technique): shaderTechnique_{technique}
{
}

void gee::Pass::add_screen_target()
{
	useSwapchainTarget_ = true;
	screenTargetIndex_ = std::size(colorTargets_);
}

void gee::Pass::add_color_target(const RenderTarget& target)
{
	colorTargets_.emplace_back(target);
}

void gee::Pass::add_depth_target(const RenderTarget& target)
{
	depthTargets_.emplace_back(target);
}

void gee::Pass::add_input_target(const RenderTarget& target)
{
	inputTargets_.emplace_back(target);
}

void gee::Pass::set_index(const size_t index)
{
	const_cast<ShaderTechnique&>(shaderTechnique_.get()).passIndex_ = index;
}