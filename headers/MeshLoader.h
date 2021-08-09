#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Geometry.h"
#include "Material.h"
#include "ResourceHolder.h"
#include "Texture.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace fs = std::filesystem;
namespace gee
{
	class StaticMesh;
	class MeshLoader
	{
	public:
		using TextureHolderRef = std::reference_wrapper<gee::ResourceHolder<gee::Texture>>;

		MeshLoader(gee::ResourceHolder<gee::Texture>& textureHolder, const int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
			| aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices | aiProcess_GenNormals);
		MeshLoader(const int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
			| aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices | aiProcess_GenNormals);
		StaticMesh load(const std::string& path);
	private:
		int flags_;
		fs::path path_;
		std::vector<gee::Vertex> vertices_;
		std::vector<uint32_t> indices_;
		gee::Material material_;
		std::optional<TextureHolderRef> textureHolder_{ std::nullopt };
		void processNode(const aiScene* scene, const aiNode* node);
		void processMesh(const aiScene* scene, const aiMesh* mesh);
		void processMaterial(const aiMaterial* material);
		const std::string getTexturePath(const aiMaterial& material, const aiTextureType type, const std::string& defaultTexPath) const;
	};
}