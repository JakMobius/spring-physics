
#include "terrain.hpp"

std::unique_ptr<TerrainChunk> Terrain::create_chunk() {
    auto chunk = std::make_unique<TerrainChunk>(this);
    m_chunks.insert(chunk.get());
    m_is_valid = false;
    return chunk;
}

void Terrain::destroy_chunk(TerrainChunk* chunk) {
    chunk->set_terrain(nullptr);
    m_chunks.erase(chunk);
    m_is_valid = false;
}

void Terrain::query(const Vec3f& from, const Vec3f& to, std::vector<TerrainPolygon*>& result) {
    m_chunk_vector.clear();
    m_bvh.query(m_bvh.m_root.get(), Vec3f::min(from, to), Vec3f::max(from, to), m_chunk_vector);

    for (auto& chunk : m_chunk_vector) {
        chunk->query(from, to, result);
    }
}

void Terrain::ensure_valid() {
    for (auto& chunk : m_chunks) {
        if(!chunk->is_valid()) {
            m_is_valid = false;
            chunk->ensure_valid();
        }
    }

    if (m_is_valid) {
        return;
    }

    m_chunk_vector.assign(m_chunks.begin(), m_chunks.end());
    m_bvh.build(m_chunk_vector);
    m_is_valid = true;
}
