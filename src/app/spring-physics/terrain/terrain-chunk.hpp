#pragma once

class TerrainChunk;
class World;

#include "terrain-bvh.hpp"
#include "terrain.hpp"
#include <vector>

class TerrainChunk {
    std::vector<std::unique_ptr<TerrainPolygon>> m_surface_mesh{};
    Terrain* m_terrain;

    TerrainBVH m_bvh;
    bool m_bvh_valid = false;

  public:
    TerrainChunk(Terrain* terrain)
        : m_terrain(terrain) {}
    ~TerrainChunk();

    TerrainPolygon* add_triangle(Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    void destroy_triangle(TerrainPolygon* triangle);

    const std::vector<std::unique_ptr<TerrainPolygon>>& get_surface_mesh();

    void ensure_valid();
    bool is_valid() { return m_bvh_valid; }

    void query(const Vec3f& from, const Vec3f& to, std::vector<TerrainPolygon*>& result);

    void set_terrain(Terrain* terrain) {
        m_terrain = terrain;
    }

    Terrain* get_terrain() {
        return m_terrain;
    }

    TerrainBVH& get_bvh() {
        return m_bvh;
    }
};