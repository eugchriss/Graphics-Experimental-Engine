#pragma once
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include "Sampler.h"
#include "Texture.h"

namespace gee
{
    template<class T>
    struct is_array : std::is_array<T> {};
    template<class T>
    struct is_array<std::vector<T>> : std::true_type{};

    class ShaderValue
    {
    public:
        template<class T>
        ShaderValue(const std::string& name, T& datas) : name_{ name }, datas_(std::make_shared<std::any>(datas))
        {
            if constexpr (is_array<T>::value)
            {
                address_ = std::data(std::any_cast<decltype(datas)>(*datas_));
                size_ = std::size(datas) * sizeof(datas[0]);
            }
            else
            {
                address_ = std::any_cast<T>(datas_.get());
                size_ = sizeof(T);
            }
        }
        void* datas() const
        {
            return address_;
        }
        size_t size() const
        {
            return size_;
        }
        const std::string& name() const
        {
            return name_;
        }
    private:
        std::string name_;
        std::shared_ptr<std::any> datas_;
        void* address_;
        size_t size_;
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
        void add_empty_texture()
        {
            textures.emplace_back(std::nullopt);
        }
    };
}