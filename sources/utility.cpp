#include "../headers/utility.h"

gee::Floor::Floor()
{
	assert(std::size(indices) % 3 == 0 && "Drawing primitives should be triangles");

	for (auto i = 0u; i < std::size(indices); i += 3)
	{
        auto& v0 = vertices[indices[i]];
        auto& v1 = vertices[indices[i + 1]];
        auto& v2 = vertices[indices[i + 2]];

        auto Edge1 = v1.position - v0.position;
        auto Edge2 = v2.position - v0.position;

        float DeltaU1 = v1.textureCoord.x - v0.textureCoord.x;
        float DeltaV1 = v1.textureCoord.y - v0.textureCoord.y;
        float DeltaU2 = v2.textureCoord.x - v0.textureCoord.x;
        float DeltaV2 = v2.textureCoord.y - v0.textureCoord.y;

        float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

        glm::vec3 tangent;
        tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
        tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
        tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

        glm::vec3 bitangent;
        bitangent.x = f * (-DeltaU2 * Edge1.x - DeltaU1 * Edge2.x);
        bitangent.y = f * (-DeltaU2 * Edge1.y - DeltaU1 * Edge2.y);
        bitangent.z = f * (-DeltaU2 * Edge1.z - DeltaU1 * Edge2.z);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
	}

    for (auto& vertex : vertices)
    {
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::cross(vertex.tangent, vertex.normal);
    }
}