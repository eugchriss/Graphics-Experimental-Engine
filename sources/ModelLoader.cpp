#include "../headers/ModelLoader.h"
#include <cassert>

gee::ModelLoader::ModelLoader(unsigned int flags) : flags_{ flags }
{
}

gee::Mesh gee::ModelLoader::load(const std::string& path)
{
	meshPath_ = path;
	Assimp::Importer importer{};
	auto scene = importer.ReadFile(path, flags_);
	if (!scene)
	{
		auto error = std::string{ importer.GetErrorString() };
		throw std::runtime_error{ "Assimp loader failed: " + error };
	}
	processNode(scene, scene->mRootNode);
	assert(!std::empty(textures_) && "The mesh doesn t have any texture attached");
	return Mesh{scene->GetShortFilename(path.c_str()), std::move(vertices_), std::move(indices_), std::move(textures_) };
}

void gee::ModelLoader::processNode(const aiScene* scene, const aiNode* node)
{
	for (auto i = 0u; i < node->mNumMeshes; ++i)
	{
		processMesh(scene, scene->mMeshes[node->mMeshes[i]]);
	}
	for (auto i = 0u; i < node->mNumChildren; ++i)
	{
		processNode(scene, node->mChildren[i]);
	}
}

void gee::ModelLoader::processMesh(const aiScene* scene, const aiMesh* mesh)
{
	for (auto i = 0u; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		vertex.position = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (mesh->HasVertexColors(0))
		{
			vertex.color = glm::vec3{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
		}
		if (mesh->HasTextureCoords(0))
		{
			vertex.textureCoord = glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		if (mesh->HasNormals())
		{
			vertex.normal = glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		}

		vertices_.push_back(vertex);
	}
	for (auto i = 0u; i < mesh->mNumFaces; ++i)
	{
		for (auto j = 0u; j < mesh->mFaces[i].mNumIndices; ++j)
		{
			indices_.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}
	if (mesh->mMaterialIndex >= 0)
	{
		processMaterial(scene->mMaterials[mesh->mMaterialIndex]);
	}
}

void gee::ModelLoader::processMaterial(const aiMaterial* material)
{
	std::vector<aiTextureType> textureTypes{ aiTextureType_DIFFUSE, aiTextureType_HEIGHT, aiTextureType_SPECULAR };
	for (auto texture_t : textureTypes)
	{
		auto textCount = material->GetTextureCount(texture_t);
		if (textCount > 0)
		{
			aiString relativeTexturePath;
			material->GetTexture(texture_t, 0, &relativeTexturePath);
			fs::path texturePath{ meshPath_.parent_path() };
			texturePath += "/";
			texturePath += relativeTexturePath.C_Str();
			textures_.emplace(std::make_pair(texture_t, texturePath.string()));
		}
	}
}