#pragma once

class TerrainChunk;

#include "../../../utils/bvh.hpp"
#include "../../../utils/vec3.hpp"

struct TerrainChunkBVH : public LazyBVH<TerrainChunk*, TerrainChunkBVH> {

    static Vec3f get_min(TerrainChunk& triangle);
    static Vec3f get_max(TerrainChunk& triangle);
    static Vec3f lower_bound(std::vector<TerrainChunk*>::iterator begin, std::vector<TerrainChunk*>::iterator end);
    static Vec3f upper_bound(std::vector<TerrainChunk*>::iterator begin, std::vector<TerrainChunk*>::iterator end);
    static bool compare(TerrainChunk* a, TerrainChunk* b, int axis);
};