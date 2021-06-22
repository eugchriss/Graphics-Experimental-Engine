#pragma once
#include "enums.h"
#include "utility.h"

namespace gee
{
	struct SamplerAddressMode
	{
		SamplerAddressMode(const AddressMode mode = AddressMode::REPEAT): u{mode}, v{mode}, w{mode}
		{}
		AddressMode u{};
		AddressMode v{};
		AddressMode w{};
	};

	struct Sampler
	{
		Filter magFilter{ Filter::LINEAR };
		Filter minFilter{ Filter::LINEAR };
		SamplerAddressMode addressMode{};
		Filter mipmapMode{ Filter::LINEAR };
		float mipLodBias{};
		float minLod{};
		float maxLod{};
		float maxAniostropy{ Sampler::max_aniostropy() };
		CompareOp compareOp{CompareOp::NEVER};
		BorderColor borderColor{BorderColor::OPAQUE_BLACK};

		static float max_aniostropy()
		{
			//16 is the max aniostropy for 99% of hardwares from all platforms. Source:http://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxSamplerAnisotropy&platform=windows
			return 16;
		}
	};

	template<>
	struct ID<SamplerAddressMode>
	{
		using type = size_t;
		static type get(const SamplerAddressMode& mode)
		{
			type seed{};
			hash_combine(seed, mode.u, mode.v, mode.w);
			return seed;
		}
	};

	template<>
	struct ID<Sampler>
	{
		using Type = size_t;
		static Type get(const Sampler& sampler)
		{
			Type seed{};
			hash_combine(seed, static_cast<size_t>(sampler.magFilter), static_cast<size_t>(sampler.minFilter), ID<SamplerAddressMode>::get(sampler.addressMode),
				static_cast<size_t>(sampler.mipmapMode), sampler.mipLodBias, sampler.minLod, sampler.maxLod, sampler.maxAniostropy,
				static_cast<size_t>(sampler.compareOp), static_cast<size_t>(sampler.borderColor));
			return seed;
		}
	};
}

