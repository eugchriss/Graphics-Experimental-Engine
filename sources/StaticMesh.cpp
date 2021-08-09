#include "..\headers\StaticMesh.h"

const gee::Geometry& gee::StaticMesh::geometry() const
{
    return geometry_;
}

const gee::Material& gee::StaticMesh::material() const
{
    return material_;
}

gee::StaticMesh::StaticMesh(gee::Geometry&& geometry, gee::Material&& material, const std::string& name):
    geometry_{std::move(geometry)}, material_{std::move(material)}, name_{name}
{
}
