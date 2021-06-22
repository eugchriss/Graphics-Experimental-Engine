#pragma once
#include <string>

#include "utility.h"
namespace gee
{
	namespace vkn { class Pipeline; }

	template<class T> struct ResourceLoader;
	class ShaderTechnique
	{
	public:
		ShaderTechnique(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		const std::string& vertexShaderPath() const;
		const std::string& geometryShaderPath() const;
		const std::string& fragmentShaderPath() const;
		void wireframe_on(const bool value);
	private:
		friend class Pass;
		friend ResourceLoader<vkn::Pipeline>;
		bool hasChanged_{ true };
		std::string vertexShaderPath_{};
		std::string geometryShaderPath_{};
		std::string fragmentShaderPath_{};
		bool useWireFrame_{ false };
		size_t passIndex_{};

		friend ID<ShaderTechnique>;
	};
	template<>
	struct ID<ShaderTechnique>
	{
		using Type = size_t;
		static auto get(const ShaderTechnique& m)
		{
			size_t seed{};
			hash_combine(seed, std::hash<std::string>{}(m.vertexShaderPath_), std::hash<std::string>{}(m.fragmentShaderPath_));
			return seed;
		}
	};
	MAKE_CONST_REFERENCE(ShaderTechnique);
}
