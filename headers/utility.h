#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "Mesh.h"

namespace gee
{
	void processTangentSpace(std::vector<gee::Vertex>& vertices, std::vector<uint32_t>& indices);
	gee::Mesh getCubeMesh(gee::Material&& material = gee::Material{});
	gee::Mesh getQuadMesh(gee::Material&& material = gee::Material{});
	gee::Mesh getFloorMesh(gee::Material&& material = gee::Material{});
}

