#pragma once
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

#include "utility.h"
namespace gee
{
	namespace vkn { class Pipeline; }

	template<class T> struct ResourceLoader;
	class ShaderTechnique
	{
		using Set = uint32_t;
		using Alignments = std::vector<size_t>;
	public:
		ShaderTechnique(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		const std::string& vertexShaderPath() const;
		const std::string& geometryShaderPath() const;
		const std::string& fragmentShaderPath() const;
		void set_dynamic_alignments(const Set set, const Alignments& alignments);
		const std::unordered_map<Set, Alignments>& dynamic_alignments() const;
	private:
		friend class Pass;
		friend ResourceLoader<vkn::Pipeline>;
		bool isGPUInitialized_{ false };
		std::string vertexShaderPath_{};
		std::string geometryShaderPath_{};
		std::string fragmentShaderPath_{};
		bool useWireFrame_{ false };
		size_t passIndex_{};
		std::unordered_map<Set, Alignments> setDynamicAlignments_;
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
