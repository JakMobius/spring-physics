
#include "material.hpp"
#include "geometry-pool.hpp"

void Material::set_color(const Vec3f &color) {
    m_color = color;
    m_geometry_pool->update_material_delayed(this);
}

void Material::set_grid(bool grid) {
    m_grid = grid;
    m_geometry_pool->update_material_delayed(this);
}

void Material::set_specular_factor(float factor) {
    m_specular_factor = factor;
    m_geometry_pool->update_material_delayed(this);
}

Material::~Material() {
    if(m_geometry_pool) {
        m_geometry_pool->destroy_material(this);
    }
}
