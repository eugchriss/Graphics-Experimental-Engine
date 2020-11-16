#include "..\headers\AABB.h"
#include "glm/gtx/transform.hpp"
#include <algorithm>

gee::AABB::AABB(const std::vector<gee::Vertex>& vertices)
{
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> zs;
    xs.reserve(std::size(vertices));
    ys.reserve(std::size(vertices));
    zs.reserve(std::size(vertices));

    for (const auto& vertex : vertices)
    {
        xs.push_back(vertex.position.x);
        ys.push_back(vertex.position.y);
        zs.push_back(vertex.position.z);
    }
    std::sort(std::begin(xs), std::end(xs));
    std::sort(std::begin(ys), std::end(ys));
    std::sort(std::begin(zs), std::end(zs));
    origin_.x = xs[0];
    origin_.y = ys[0];
    origin_.z = zs[0];
    extend_.x = xs.back();
    extend_.y = ys.back();
    extend_.z = zs.back();

    auto offset = 0.5f * (extend_ + origin_);
    transformMatrix = glm::translate(transformMatrix, offset);
    auto width = extend_.x - origin_.x;
    auto height = extend_.y - origin_.y;
    auto depth = extend_.z - origin_.z;
    if (width == 0.0f)
    {
        width = 0.00002f;
    }
    if (height == 0.0f)
    {
        height = 0.00002f;
    }
    if (depth == 0.0f)
    {
        depth = 0.00002f;
    }
    glm::vec3 scale{};
    scale.x = 0.5f * width;
    scale.y = 0.5f * height;
    scale.z = 0.5f * depth;
    transformMatrix = glm::scale(transformMatrix, scale);
}

const std::vector<gee::Vertex>& gee::AABB::vertices() const
{
    return mesh_.vertices();
}

const std::vector<uint32_t>& gee::AABB::indices() const
{
    return mesh_.indices();
}

const gee::AbstractMesh& gee::AABB::mesh() const
{
    return mesh_;
}
