
#include "terrain-chunk.hpp"

TerrainPolygon* TerrainChunk::add_triangle(Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c) {
    auto triangle = std::make_unique<TerrainPolygon>(vertex_a, vertex_b, vertex_c);
    auto triangle_ptr = triangle.get();
    m_surface_mesh.push_back(std::move(triangle));
    m_bvh_valid = false;
    return triangle_ptr;
}

void TerrainChunk::destroy_triangle(TerrainPolygon* triangle) {
    auto it = std::find_if(m_surface_mesh.begin(), m_surface_mesh.end(), [triangle](auto& t) {
        return t.get() == triangle; });
    if (it != m_surface_mesh.end()) {
        m_surface_mesh.erase(it);
        m_bvh_valid = false;
    }
}

const std::vector<std::unique_ptr<TerrainPolygon>>& TerrainChunk::get_surface_mesh() {
    return m_surface_mesh;
}

void TerrainChunk::ensure_valid() {
    if (m_bvh_valid) {
        return;
    }
    std::vector<TerrainPolygon*> triangles;
    triangles.reserve(m_surface_mesh.size());
    for (auto& triangle : m_surface_mesh) {
        triangles.push_back(triangle.get());
    }
    m_bvh.build(triangles);
    m_bvh_valid = true;
}

void TerrainChunk::query(const Vec3f& from, const Vec3f& to, std::vector<TerrainPolygon*>& result) {
    assert(m_bvh_valid);
    m_bvh.query(m_bvh.m_root.get(), Vec3f::min(from, to), Vec3f::max(from, to), result);
}

TerrainChunk::~TerrainChunk() {
    if (m_terrain) {
        m_terrain->destroy_chunk(this);
    }
}
