#pragma once
#include <vector>
#include "RenderTarget.h"
#include "ShaderTechnique.h"

namespace gee
{
	class Pass
	{
	public:
		Pass(const ShaderTechnique& technique);
		void add_screen_target();
		void add_color_target(const RenderTarget& target);
		void add_depth_target(const RenderTarget& target);
		void add_input_target(const RenderTarget& target);
		
		const bool use_screen_target() const;
		const std::vector<RenderTarget>& color_targets() const;
		const std::vector<RenderTarget>& depth_targets() const;
		const std::vector<RenderTarget>& input_targets() const;
	private:
		bool useSwapchainTarget_{ false };
		std::vector<RenderTarget> colorTargets_;
		std::vector<RenderTarget> depthTargets_;
		std::vector<RenderTarget> inputTargets_;
		ShaderTechniqueConstRef shaderTechnique_;
	};

}
