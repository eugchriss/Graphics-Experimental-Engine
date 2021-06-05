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
		void set_property(const TEXTURE_SLOT slot, gee::Texture& texture)
		{
			textureSlots.emplace(slot, std::ref(texture));
		}
		void add_geometry(const gee::Geometry& geometry, const glm::mat4& transform)
		{
			auto result = geometries.find(geometry);
			if (result == std::end(geometries))
			{
				geometries[geometry].resize(1024);
				geometries[geometry][0] = transform;
				geomtriesNextTransformIndex_[geometry] = 1;
			}
			else
			{
				auto& index = geomtriesNextTransformIndex_[geometry];
				result->second[index]= transform;
				++index;
			}
		}
		void reset_transforms()
		{
			for (auto& [geometryRef, index] : geomtriesNextTransformIndex_)
			{
				index = 0;
			}
		}
		template<class Iterator>
		size_t copy_geometries_to(Iterator dst)
		{
			auto geometryOffset = 0u;
			const auto transformSize = vkn::Material::max_object_per_instance();
			for (const auto& [geometry, transforms] : geometries)
			{
				std::copy_n(std::begin(transforms), geomtriesNextTransformIndex_[geometry], dst + geometryOffset * transformSize);
				++geometryOffset;
			}
			return geometryOffset * transformSize;
		}
		std::unordered_map<gee::GeometryConstRef, std::vector<glm::mat4>> geometries;
		std::unordered_map<gee::GeometryConstRef, size_t> geomtriesNextTransformIndex_;
		std::unordered_map<TEXTURE_SLOT, TextureRef> textureSlots;
		gee::MaterialRef materialRef_;
	};
	MAKE_REFERENCE(MaterialInstance)
}
