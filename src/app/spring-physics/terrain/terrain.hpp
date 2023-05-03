#pragma once

#include <vector>
#include "../objects/surface-triangle-object.hpp"
#include "terrain-bvh.hpp"

class World;

class Terrain {
    std::vector<TerrainPolygon*> m_surface_mesh {};

    TerrainBVH m_bvh;
    bool bvh_valid = false;

public:
    Terrain() {}

    std::unique_ptr<TerrainPolygon> add_triangle(Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    void destroy_triangle(TerrainPolygon* triangle);

    std::vector<TerrainPolygon*>& get_surface_mesh();

    void ensure_valid();

    void query(const Vec3f& from, const Vec3f& to, std::vector<TerrainPolygon*>& result);
};