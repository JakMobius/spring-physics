
#include "terrain-chunk-bvh.hpp"
#include "terrain-chunk.hpp"

Vec3f TerrainChunkBVH::get_min(TerrainChunk& chunk) {
    return chunk.get_bvh().m_root->min;
}

Vec3f TerrainChunkBVH::get_max(TerrainChunk& chunk) {
    return chunk.get_bvh().m_root->max;
}

Vec3f TerrainChunkBVH::upper_bound(std::vector<TerrainChunk*>::iterator begin, std::vector<TerrainChunk*>::iterator end) {
    Vec3f result = get_max(**begin);
    for (auto it = begin; it != end; ++it) {
        result = Vec3f::max(result, get_max(**it));
    }
    return result;
}

Vec3f TerrainChunkBVH::lower_bound(std::vector<TerrainChunk*>::iterator begin, std::vector<TerrainChunk*>::iterator end) {
    Vec3f result = get_min(**begin);
    for (auto it = begin; it != end; ++it) {
        result = Vec3f::min(result, get_min(**it));
    }
    return result;
}

bool TerrainChunkBVH::compare(TerrainChunk* a, TerrainChunk* b, int axis) {
    auto& a_bvh = a->get_bvh().m_root;
    auto& b_bvh = b->get_bvh().m_root;

    return a_bvh->min[axis] + a_bvh->max[axis] < b_bvh->min[axis] + b_bvh->max[axis];
}