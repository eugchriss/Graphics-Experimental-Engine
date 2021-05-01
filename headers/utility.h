#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "Mesh.h"

namespace gee
{
	void processTangentSpace(std::vector<gee::Vertex>& vertices, std::vector<uint32_t>& indices);
	gee::Geometry getCubeGeometry(); 
	gee::Geometry getQuadGeometry();
	gee::Geometry getFloorGeometry();
}

