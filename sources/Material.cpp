#include "../headers/Material.h"

gee::IdDispenser<size_t> gee::Material::idDispenser_{};

gee::Material::Material()
{
	id_ = idDispenser_.get();
}

gee::Material::Material(Material&& other)
{
	id_ = other.id_;
	properties_ = std::move(other.properties_);
	shouldDeleteID_ = true;
	other.shouldDeleteID_ = false;
}
gee::Material::~Material()
{
	if (shouldDeleteID_)
	{
		idDispenser_.consumed(id_);
	}
}

void gee::Material::set_property(const MaterialProperty& property, const Texture& texture)
{
	properties_.emplace(property, texture);
}

bool gee::Material::operator==(const Material& other) const
{
	return id_ == other.id_;
}
