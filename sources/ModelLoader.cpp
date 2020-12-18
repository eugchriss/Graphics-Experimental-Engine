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
	return Mesh{ scene->GetShortFilename(path.c_str()), std::move(vertices_), std::move(indices_), std::move(material_) };
}

gee::Mesh gee::ModelLoader::create(const std::string& path)
{
	return load(path);
}

gee::Mesh gee::ModelLoader::create(std::function<gee::Mesh(gee::Material&&)> customMesh, gee::Material material)
{
	return customMesh(std::move(material));
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
			vertex.color = glm::vec3{ mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b };
		}
		if (mesh->HasTextureCoords(0))
		{
			vertex.textureCoord = glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		if (mesh->HasNormals())
		{
			vertex.normal = glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent = glm::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
				vertex.bitangent = glm::vec3{ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			}
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

void gee::ModelLoader::processMaterial(const aiMaterial* mat)
{
	const auto& diffuseTexPath = getTexturePath(*mat, aiTextureType_DIFFUSE, "../assets/default_textures/diffuse.png");
	const auto& normalTexPath = getTexturePath(*mat, aiTextureType_HEIGHT, "../assets/default_textures/normal.png");
	const auto& specularTexPath = getTexturePath(*mat, aiTextureType_SPECULAR, "../assets/default_textures/specular.png");
	material_ = gee::Material{ diffuseTexPath, normalTexPath, specularTexPath };
}

const std::string gee::ModelLoader::getTexturePath(const aiMaterial& material, const aiTextureType type, const std::string& defaultTexPath) const
{
	const auto textCount = material.GetTextureCount(type);
	if (textCount == 0)
	{
		return defaultTexPath;
	}
	else if (textCount > 1)
	{
		throw std::runtime_error{ "This loader doesn t support multi material meshes" };
	}

	aiString relativeTexturePath;
	//only using the first texture for now
	material.GetTexture(type, 0, &relativeTexturePath);
	fs::path texturePath{ meshPath_.parent_path() };
	texturePath += "/";
	texturePath += relativeTexturePath.C_Str();
	return texturePath.string();
}
