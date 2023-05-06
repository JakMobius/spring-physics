
#include "terrain-bvh.hpp"

Vec3f TerrainBVH::get_min(TerrainPolygon& triangle) {
    Vec3f result = Vec3f::min(triangle.get_point_a(), triangle.get_point_b());
    result = Vec3f::min(result, triangle.get_point_c());
    result -= Vec3f(triangle.get_thickness(), triangle.get_thickness(), triangle.get_thickness());
    return result;
}

Vec3f TerrainBVH::get_max(TerrainPolygon& triangle) {
    Vec3f result = Vec3f::max(triangle.get_point_a(), triangle.get_point_b());
    result = Vec3f::max(result, triangle.get_point_c());
    result += Vec3f(triangle.get_thickness(), triangle.get_thickness(), triangle.get_thickness());
    return result;
}

Vec3f TerrainBVH::upper_bound(std::vector<TerrainPolygon*>::iterator begin, std::vector<TerrainPolygon*>::iterator end) {
    Vec3f result = get_max(**begin);
    for (auto it = begin; it != end; ++it) {
        result = Vec3f::max(result, get_max(**it));
    }
    return result;
}

Vec3f TerrainBVH::lower_bound(std::vector<TerrainPolygon*>::iterator begin, std::vector<TerrainPolygon*>::iterator end) {
    Vec3f result = get_min(**begin);
    for (auto it = begin; it != end; ++it) {
        result = Vec3f::min(result, get_min(**it));
    }
    return result;
}

bool TerrainBVH::compare(TerrainPolygon* a, TerrainPolygon* b, int axis){
    float a_axis = a->get_point_a()[axis] + a->get_point_b()[axis] + a->get_point_c()[axis];
    float b_axis = b->get_point_a()[axis] + b->get_point_b()[axis] + b->get_point_c()[axis];
    return a_axis < b_axis;
}