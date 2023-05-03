#pragma once

class GeometryPool;

#include "../utils/vec3.hpp"

class Material {
  private:
    GeometryPool* m_geometry_pool;
    int m_buffer_index = 0;
    bool m_grid = false;
    float m_specular_factor = 10;
    Vec3f m_color = {1, 1, 1};

  public:
    Material(GeometryPool* pool)
        : m_geometry_pool(pool) {}
    ~Material();

    GeometryPool* get_geometry_pool() {
        return m_geometry_pool;
    }
    void set_geometry_pool(GeometryPool* pool) {
        m_geometry_pool = pool;
    }

    Vec3f get_color() {
        return m_color;
    }
    void set_color(const Vec3f& color);

    bool get_grid() {
        return m_grid;
    }
    void set_grid(bool value);

    int get_buffer_index() {
        return m_buffer_index;
    }
    void set_buffer_index(int index) {
        m_buffer_index = index;
    }

    float get_specular_factor() {
        return m_specular_factor;
    }
    void set_specular_factor(float factor);
};