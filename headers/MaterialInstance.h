#pragma once
#include "enums.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

#include "impl/vulkan/Material.h"
namespace gee
{
	class MaterialInstance
	{
	public:
		MaterialInstance(gee::Material& material) : materialRef_{material}
		{
		}
		MaterialInstance(MaterialInstance&& other) = default;
		std::unordered_map<TEXTURE_SLOT, TextureRef> textureSlots;
		gee::MaterialRef materialRef_;
	};
	MAKE_REFERENCE(MaterialInstance)
}
