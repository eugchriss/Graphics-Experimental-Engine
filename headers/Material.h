#pragma once
#include <string>

#include "utility.h"
namespace gee
{
	class Material
	{
	public:
		Material(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		const std::string& vertexShaderPath() const;
		const std::string& geometryShaderPath() const;
		const std::string& fragmentShaderPath() const;
		void wireframe_on(const bool value);
	private:
		bool hasChanged_{ true };
		std::string vertexShaderPath_{};
		std::string geometryShaderPath_{};
		std::string fragmentShaderPath_{};
		bool useWireFrame_{ false };

		friend ID<Material>;
	};
	template<>
	struct ID<Material>
	{
		using Type = size_t;
		static auto get(const Material& m)
		{
			size_t seed{};
			hash_combine(seed, std::hash<std::string>{}(m.vertexShaderPath_), std::hash<std::string>{}(m.fragmentShaderPath_));
			return seed;
		}
	};
	MAKE_REFERENCE(Material);
}
