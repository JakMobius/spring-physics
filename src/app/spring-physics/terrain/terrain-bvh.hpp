#pragma once

#include "../../../utils/bvh.hpp"
#include "../../../utils/vec3.hpp"
#include "../physics/terrain-polygon.hpp"
#include <vector>

struct TerrainBVH : public LazyBVH<TerrainPolygon*, TerrainBVH> {

    static Vec3f get_min(TerrainPolygon& triangle);
    static Vec3f get_max(TerrainPolygon& triangle);
    static Vec3f lower_bound(std::vector<TerrainPolygon*>::iterator begin, std::vector<TerrainPolygon*>::iterator end);
    static Vec3f upper_bound(std::vector<TerrainPolygon*>::iterator begin, std::vector<TerrainPolygon*>::iterator end);
    static bool compare(TerrainPolygon* a, TerrainPolygon* b, int axis);
};