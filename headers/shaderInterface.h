#pragma once
#include <optional>
#include <string>
#include <utility>
#include "Sampler.h"
#include "Texture.h"

namespace gee
{
    struct ShaderValue
    {
        std::string name;
        void* address;
        size_t size;
    };
    struct ShaderTexture
    {
        std::string name;
        Sampler sampler;
        std::reference_wrapper<const Texture> texture;
    };
    struct ShaderArrayTexture
    {
        std::string name;
        Sampler sampler;
        std::vector<std::optional<std::reference_wrapper<const Texture>>> textures;
        void add(const Texture& texture)
        {
            textures.emplace_back(texture);
        }
    };
}