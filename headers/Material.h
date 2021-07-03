#pragma once
#include <unordered_map>

#include "ShaderTechnique.h"
#include "Texture.h"
#include "utility.h"
namespace gee
{
	enum class MaterialProperty
	{
		COLOR,
		NORMAL,
		SPECULAR
	};
	class Material
	{
	public:
		Material();
		~Material();
		void set_property(const MaterialProperty& property, const Texture& texture);
		const std::unordered_map<MaterialProperty, std::reference_wrapper<const Texture>>& properties() const
		{
			return properties_;
		}
		bool operator==(const Material& other) const;
	private:
		friend ID<Material>;
		static gee::IdDispenser<size_t> idDispenser_;
		size_t id_;
		std::unordered_map<MaterialProperty, std::reference_wrapper<const Texture>> properties_;
	};

	template<>
	struct ID<Material>
	{
		using Type = size_t;
		static auto get(const Material& material)
		{
			return material.id_;
		}
	};
	MAKE_REFERENCE(Material);
}
