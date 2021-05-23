#include <fstream>
#include <stdexcept>
#include <utility>

#include "../../headers/impl/vulkan/pass.h"
#include "../../headers/impl/vulkan/Renderpass.h"
#include "../../headers/impl/vulkan/vulkan_utils.h"

using namespace gee;
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