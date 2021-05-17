#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../../headers/Texture.h"

gee::Texture::Texture(const std::string& name, const std::string& path, const ColorSpace colorSpace) : name_{ name }, paths_ { path }, colorSpace_{ colorSpace }
{
	int channel{};
	int width, height;
	auto pixels = stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);
	assert(pixels);
	width_ = width;
	height_ = height;
	auto textureSize = width * height * 4;

	pixels_.resize(textureSize);
	std::copy_n(pixels, textureSize, std::begin(pixels_));
	stbi_image_free(pixels);

	offsets_.emplace_back(0);
}

gee::Texture::Texture(const std::array<std::string, 6>& paths, const ColorSpace colorSpace) : colorSpace_ { colorSpace }
{
	std::array<std::tuple<int, int, int>, 6> formats{};
	std::array<stbi_uc*, 6> pixels;

	for (auto i = 0u; i < 6; ++i)
	{
		paths_.emplace_back(paths[i]);
		pixels[i] = stbi_load(paths[i].c_str(), &std::get<0>(formats[i]), &std::get<1>(formats[i]), &std::get<2>(formats[i]), STBI_rgb_alpha);
	}
	const auto& reference = formats[0];
	size_t textureSize{};
	for (const auto& [width, height, channel] : formats)
	{
		assert(width == std::get<0>(reference) && height == std::get<1>(reference) && "All textures need to have the exact same size");
		textureSize += width * height * 4;
	}
	width_ = std::get<0>(reference);
	height_ = std::get<1>(reference);
	pixels_.resize(textureSize);
	size_t offset{};
	for (auto i = 0u; i < 6; ++i)
	{
		auto size = std::get<0>(formats[i]) * std::get<1>(formats[i]) * 4;
		assert(pixels[i]);
		std::copy_n(pixels[i], size, std::begin(pixels_) + offset);
		offsets_.emplace_back(offset);
		offset += size;
		stbi_image_free(pixels[i]);
	}

}

const std::vector<unsigned char>& gee::Texture::pixels() const
{
	return pixels_;
}

const std::string& gee::Texture::name() const
{
	return name_;
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

const std::vector<size_t>& gee::Texture::offsets() const
{
	return offsets_;
}
