#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../../headers/enums.h"
#include "../../headers/shaderInterface.h"

#include "spirv_cross/spirv_glsl.hpp"
#include "vulkan/vulkan.hpp"

namespace
{
	template<class T>
	struct is_array : std::false_type {};

	template <class T>
	struct is_array<std::vector<T>> : std::true_type {};
}

namespace gee
{
	template<typename T>
	class Observer_ptr
	{
	public:
		Observer_ptr() = default;
		Observer_ptr(T& resource) : ptr_{ &resource } {}
		Observer_ptr& operator= (const Observer_ptr<T>& other)
		{
			ptr_ = other.ptr_;
			return *this;
		}
		operator bool() const
		{
			return ptr_ != nullptr;
		}
		void reset(T* ptr)
		{
			ptr_ = ptr;
		}
		T* operator->()
		{
			if (!ptr_)
			{
				throw std::runtime_error{ "Nullptr dereferencing" };
			}
			return ptr_;
		}
		T* operator->() const
		{
			if (!ptr_)
			{
				throw std::runtime_error{ "Nullptr dereferencing" };
			}
			return ptr_;
		}
	private:
		T* ptr_;
	};
	namespace vkn
	{
		struct ShaderConstant
		{
			gee::ShaderValue value;
			size_t batchIndex{};
		};

		struct ShaderTexture
		{
			std::string name;
			VkImageView view;
			VkSampler sampler{ VK_NULL_HANDLE };
		};
		
		struct ShaderArrayTexture
		{
			std::string name;
			std::vector<VkImageView> views;
			VkSampler sampler{ VK_NULL_HANDLE };
		};
		void error_check(const VkResult result, const std::string& msg);
		struct Format
		{
			VkFormat format{ VK_FORMAT_UNDEFINED };
			uint32_t range{};
		};
		const Format getFormat(const spirv_cross::SPIRType& type);

		class Mapping
		{
		public:
			static VkFormat format(const gee::TargetFormat& format)
			{
				auto result = formats_.find(format);
				assert(result != std::end(formats_) && "No matching format");
				return result->second;
			}
			static VkImageUsageFlags usage(const gee::TargetUsage& usage)
			{
				auto result = usages_.find(usage);
				assert(result != std::end(usages_) && "No matching usage");
				return result->second;
			}
			static VkImageLayout layout(const gee::TargetUsage& usage)
			{
				auto result = layouts_.find(usage);
				assert(result != std::end(layouts_) && "No matching usage");
				return result->second;
			}

			static VkFilter filter(const gee::Filter& filter)
			{
				auto result = filters_.find(filter);
				assert(result != std::end(filters_) && "No matching filter");
				return result->second;
			}
			static VkSamplerAddressMode addressMode(const gee::AddressMode mode)
			{
				auto result = addressMode_.find(mode);
				assert(result != std::end(addressMode_) && "No matching address mode");
				return result->second;
			}
			static VkBorderColor borderColor(const gee::BorderColor borderColor)
			{
				auto result = borderColor_.find(borderColor);
				assert(result != std::end(borderColor_) && "No matching border color");
				return result->second;
			}
			static VkCompareOp compareOp(const gee::CompareOp op)
			{
				auto result = compareOp_.find(op);
				assert(result != std::end(compareOp_) && "No matching compare op");
				return result->second;
			}
			static VkSamplerMipmapMode mipmapMode(const gee::Filter& filter)
			{
				auto result = mipmapModes_.find(filter);
				assert(result != std::end(mipmapModes_) && "No matching compare op");
				return result->second;
			}
		public:
			static std::unordered_map<gee::TargetFormat, VkFormat> formats_;
			static std::unordered_map<gee::TargetUsage, VkImageUsageFlags> usages_;
			static std::unordered_map<gee::TargetUsage, VkImageLayout> layouts_;
			static std::unordered_map<gee::Filter, VkFilter> filters_;
			static std::unordered_map<gee::Filter, VkSamplerMipmapMode> mipmapModes_;
			static std::unordered_map<gee::AddressMode, VkSamplerAddressMode> addressMode_;
			static std::unordered_map<gee::BorderColor, VkBorderColor> borderColor_;
			static std::unordered_map<gee::CompareOp, VkCompareOp> compareOp_;

		};
	}
}