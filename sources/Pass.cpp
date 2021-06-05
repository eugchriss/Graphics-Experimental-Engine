#include "../headers/Pass.h"

gee::Pass::Pass(const ShaderTechnique& technique): shaderTechnique_{technique}
{

}

void gee::Pass::add_screen_target()
{
	useSwapchainTarget_ = true;
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

const std::vector<gee::RenderTarget>& gee::Pass::color_targets() const
{
	return colorTargets_;
}

const std::vector<gee::RenderTarget>& gee::Pass::depth_targets() const
{
	return depthTargets_;
}

const std::vector<gee::RenderTarget>& gee::Pass::input_targets() const
{
	return inputTargets_;
}
const bool gee::Pass::use_screen_target() const
{
	return useSwapchainTarget_;
}