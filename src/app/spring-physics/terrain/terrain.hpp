#pragma once

class Terrain;

#include "terrain-chunk-bvh.hpp"
#include "terrain-chunk.hpp"
#include <unordered_set>

class Terrain {
    std::unordered_set<TerrainChunk*> m_chunks;
    std::vector<TerrainChunk*> m_chunk_vector;

    TerrainChunkBVH m_bvh;
    bool m_is_valid = false;

  public:
    std::unique_ptr<TerrainChunk> create_chunk();
    void destroy_chunk(TerrainChunk* chunk);

    void query(const Vec3f& from, const Vec3f& to, std::vector<TerrainPolygon*>& result);
    void ensure_valid();
};