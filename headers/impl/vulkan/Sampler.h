#pragma once
#include "../../Sampler.h"
#include "vulkanContext.h"
#include "vulkan_utils.h"

namespace gee
{
	namespace vkn
	{
		class Sampler
		{
		public:
			Sampler(Context& context, const gee::Sampler& s): context_{context}
			{
				VkSamplerCreateInfo samplerInfo{};
				samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerInfo.pNext = nullptr;
				samplerInfo.flags = 0;
				samplerInfo.magFilter = vkn::Mapping::filter(s.magFilter);
				samplerInfo.minFilter = vkn::Mapping::filter(s.magFilter);
				samplerInfo.addressModeU = vkn::Mapping::addressMode(s.addressMode.u);
				samplerInfo.addressModeV = vkn::Mapping::addressMode(s.addressMode.v);
				samplerInfo.addressModeW = vkn::Mapping::addressMode(s.addressMode.w);
				samplerInfo.anisotropyEnable = VK_FALSE;
				/* TO DO: enable anisotropy device feature
				if (sampler.maxAniostropy > 0)
				{
					samplerInfo.anisotropyEnable = VK_TRUE;
					samplerInfo.maxAnisotropy = sampler.maxAniostropy;
				}*/
				samplerInfo.borderColor = vkn::Mapping::borderColor(s.borderColor);
				samplerInfo.unnormalizedCoordinates = VK_FALSE;
				samplerInfo.compareEnable = VK_FALSE;
				if (s.compareOp != gee::CompareOp::NEVER)
				{
					samplerInfo.compareEnable = VK_TRUE;
					samplerInfo.compareOp = vkn::Mapping::compareOp(s.compareOp);
				}
				samplerInfo.mipmapMode = vkn::Mapping::mipmapMode(s.mipmapMode);
				vkn::error_check(vkCreateSampler(context.device->device, &samplerInfo, nullptr, &sampler), "Unabled to create a sampler");
			}
			Sampler(Sampler&& other): context_{other.context_}
			{
				sampler = other.sampler;
				other.sampler = VK_NULL_HANDLE;
			}
			~Sampler()
			{
				if (sampler != VK_NULL_HANDLE)
				{
					vkDestroySampler(context_.device->device, sampler, nullptr);
				}
			}
			VkSampler sampler{ VK_NULL_HANDLE };
		private:
			Context& context_;
		};
	}
	template<class T> struct ResourceLoader;

	template<>
	struct ResourceLoader<vkn::Sampler>
	{
		static vkn::Sampler load(vkn::Context& context, const Sampler& sampler)
		{
			return vkn::Sampler{context, sampler};
		}
	};
}
