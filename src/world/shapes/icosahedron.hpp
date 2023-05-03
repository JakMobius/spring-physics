#pragma once

#include "../../utils/vec3.hpp"
#include <vector>

class IcosahedronShape {
  public:
    const static float golden_ratio;
    const static float icosahedron_radius;
    const static std::vector<Vec3f> vertices;
    const static std::vector<Vec3i> faces;
};