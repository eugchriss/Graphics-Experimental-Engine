#pragma once
#include <array>
#include <string>
#include <vector>

#include "utility.h"

#define MAKE_CONST_REFERENCE(object) using object##ConstRef = std::reference_wrapper<const object>;
namespace gee
{
	class Texture
	{
	public:
		enum class ColorSpace
		{
			LINEAR,
			NON_LINEAR
		};

		Texture(const std::string& path, const ColorSpace colorSpace = ColorSpace::LINEAR);
		Texture(const std::array<std::string, 6>& paths, const ColorSpace colorSpace);
		Texture(Texture&& other) = default;
		Texture& operator=(Texture&&) = default;
		const std::vector<unsigned char>& pixels() const;
		const uint32_t width() const;
		const uint32_t height() const;
		const ColorSpace colorSpace() const;
		const std::vector<size_t>& offsets() const;
		std::vector<std::string> paths_;
	private:
		uint32_t width_{};
		uint32_t height_{};
		std::vector<unsigned char> pixels_;
		std::vector<size_t> offsets_;
		ColorSpace colorSpace_{};

		friend ID<Texture>;
	};
	MAKE_REFERENCE(Texture);
	MAKE_CONST_REFERENCE(Texture);


	template<>
	struct ID<Texture>
	{
		using Type = size_t;
		static auto get(const Texture& t)
		{
			return std::hash<std::string>{}(t.paths_[0]);
		}
	};
}