#pragma once
#include <vector>
#include <string>

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

		Texture(const std::string& path, const ColorSpace colorSpace);
		Texture(Texture&& other) = default;
		Texture& operator=(Texture&&) = default;
		const std::vector<unsigned char>& pixels() const;
		const size_t hash() const;
		const uint32_t width() const;
		const uint32_t height() const;
		const ColorSpace colorSpace() const;
	private:
		uint32_t width_;
		uint32_t height_;
		std::vector<unsigned char> pixels_;
		std::string path_;
		ColorSpace colorSpace_;
	};
}