#pragma once
#include "Vertex.h"
#include <vector>

namespace gee
{
	class AbstractMesh
	{
	public:
		virtual const std::vector<gee::Vertex>& vertices() const = 0;
		virtual const std::vector<uint32_t>& indices() const = 0;
	};
}