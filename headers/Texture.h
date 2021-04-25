#pragma once
#include <vector>
#include <array>
#include <string>

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

		Texture(const std::string& name, const std::string& path, const ColorSpace colorSpace);
		Texture(const std::array<std::string, 6>& paths, const ColorSpace colorSpace);
		Texture(Texture&& other) = default;
		Texture& operator=(Texture&&) = default;
		const std::vector<unsigned char>& pixels() const;
		const std::string& name() const;
		const uint32_t width() const;
		const uint32_t height() const;
		const ColorSpace colorSpace() const;
		const std::vector<size_t>& offsets() const;
		std::vector<std::string> paths_;
	private:
		std::string name_;
		uint32_t width_{};
		uint32_t height_{};
		std::vector<unsigned char> pixels_;
		std::vector<size_t> offsets_;
		ColorSpace colorSpace_{};
	};
	MAKE_CONST_REFERENCE(Texture)
}