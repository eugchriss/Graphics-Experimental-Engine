#pragma once
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Mesh.h"
#include "Texture.h"

#include <vector>
#include <memory>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;
namespace gee
{

	class ModelLoader
	{
	public:
		ModelLoader(unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
			| aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices
			| aiProcess_GenNormals /*| aiProcess_FlipUVs*/);
		~ModelLoader() = default;
		Mesh load(const std::string& path);
	private:
		fs::path meshPath_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;
		std::unordered_map<aiTextureType, Texture> textures_;
		unsigned int flags_;

		void processNode(const aiScene* scene, const aiNode* node);
		void processMesh(const aiScene* scene, const aiMesh* mesh);
		void processMaterial(const aiMaterial* material);
	};
}