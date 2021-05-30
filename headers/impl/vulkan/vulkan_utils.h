#pragma once
#include <string>

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
	namespace vkn
	{
		void error_check(const VkResult result, const std::string& msg);
		struct Format
		{
			VkFormat format{ VK_FORMAT_UNDEFINED };
			uint32_t range{};
		};
		const Format getFormat(const spirv_cross::SPIRType& type);
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
	}
}