#include "../headers/pass.h"
#include "../headers/vulkan_utils.h"
#include "../headers/Renderpass.h"

#include <stdexcept>
#include <fstream>
#include <utility>


void vkn::Pass::usesColorTarget(RenderTarget& target)
{
	colorTargets_.emplace_back(std::ref(target));
}

void vkn::Pass::usesDepthStencilTarget(RenderTarget& target)
{
	depthStencilTargets_.emplace_back(std::ref(target));
}

void vkn::Pass::consumesTarget(RenderTarget& target)
{
	inputTargets_.emplace_back(std::ref(target));
}

std::vector<vkn::RenderTargetRef>& vkn::Pass::colorTargets()
{
	return colorTargets_;
}

std::vector<vkn::RenderTargetRef>& vkn::Pass::depthStencilTargets()
{
	return depthStencilTargets_;
}

std::vector<vkn::RenderTargetRef>& vkn::Pass::inputTargets()
{
	return inputTargets_;
}