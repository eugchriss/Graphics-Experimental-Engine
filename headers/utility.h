#pragma once
#include <vector>

#include "Mesh.h"
#include "glm/glm.hpp"

namespace gee
{
	void processTangentSpace(std::vector<gee::Vertex>& vertices, std::vector<uint32_t>& indices);
	gee::Geometry getCubeGeometry(); 
	gee::Geometry getQuadGeometry();
	gee::Geometry getFloorGeometry();
}

