#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../headers/Texture.h"

gee::Texture::Texture(const std::string& path, const ColorSpace colorSpace): path_{path}, colorSpace_{colorSpace}
{
	int channel{};
	int width, height;
	auto pixels = stbi_load(path_.c_str(), &width, &height, &channel, STBI_rgb_alpha);
	assert(pixels);
	width_ = width;
	height_ = height;
	auto textureSize = width * height * 4;

	pixels_.resize(textureSize);
	std::copy_n(pixels, textureSize, std::begin(pixels_));
	stbi_image_free(pixels);
}

const std::vector<unsigned char>& gee::Texture::pixels() const
{
	return pixels_;
}

const size_t gee::Texture::hash() const
{
	return std::hash<std::string>{}(path_);
}

const uint32_t gee::Texture::width() const
{
	return width_;
}

const uint32_t gee::Texture::height() const
{
	return height_;
}

const gee::Texture::ColorSpace gee::Texture::colorSpace() const
{
	return colorSpace_;
}
