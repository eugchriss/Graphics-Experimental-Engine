#pragma once
#include <vector>
#include <string>

namespace gee
{
	class Texture
	{
	public:
		Texture(const std::string& path);
		Texture(Texture&& other);
		const std::vector<unsigned char>& pixels() const;
		const size_t hash() const;
		const uint32_t width() const;
		const uint32_t height() const;
	private:
		uint32_t width_;
		uint32_t height_;
		std::vector<unsigned char> pixels_;
		std::string path_;
	};
}