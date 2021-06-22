#pragma once
#include <vector>
#include "RenderTarget.h"
#include "ShaderTechnique.h"

namespace gee
{
	namespace vkn { class Renderpass; }
	class Pass
	{
	public:
		Pass(const ShaderTechnique& technique);
		void add_screen_target();
		void add_color_target(const RenderTarget& target);
		void add_depth_target(const RenderTarget& target);
		void add_input_target(const RenderTarget& target);
	private:
		friend class Renderpass;
		friend struct ID<Pass>;
		friend struct ResourceLoader<vkn::Renderpass>;
		bool useSwapchainTarget_{ false };
		size_t screenTargetIndex_{};
		std::vector<RenderTarget> colorTargets_;
		std::vector<RenderTarget> depthTargets_;
		std::vector<RenderTarget> inputTargets_;
		ShaderTechniqueConstRef shaderTechnique_;

		void set_index(const size_t index);
	};
	template<class T> struct ID;
	template<>
	struct ID<Pass>
	{
		using Type = size_t;
		static Type get(const Pass& pass)
		{
			Type seed{};
			for (auto& target : pass.colorTargets_)
			{
				hash_combine(seed, ID<RenderTarget>::get(target));
			}
			for (auto& target : pass.depthTargets_)
			{
				hash_combine(seed, ID<RenderTarget>::get(target));
			}
			for (auto& target : pass.inputTargets_)
			{
				hash_combine(seed, ID<RenderTarget>::get(target));
			}
			return seed;
		}
	};
}
