#pragma once
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Texture.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "impl/vulkan/Material.h"


namespace fs = std::filesystem;
namespace gee
{

	class ModelLoader
	{
	public:
		ModelLoader(unsigned int flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace
			| aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices | aiProcess_GenNormals);
		~ModelLoader() = default;
		ModelLoader(ModelLoader&&) = default;
		Mesh load(const std::string& path);
		Mesh create(const std::string& path);
		Mesh create(std::function<Mesh()> customMesh);
	private:
		fs::path meshPath_;
		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;
		unsigned int flags_;

		void processNode(const aiScene* scene, const aiNode* node);
		void processMesh(const aiScene* scene, const aiMesh* mesh);
		void processMaterial(const aiMaterial* material);

		const std::string getTexturePath(const aiMaterial& material, const aiTextureType type, const std::string& defaultTexPath) const;
	};
}