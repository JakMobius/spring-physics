#pragma once

#include "world-object.hpp"
#include "../physics/vertex.hpp"
#include "../../../world/geometry-object.hpp"
#include "../../../world/shape-generator.hpp"
#include "../physics/surface.hpp"
#include "../physics/terrain-polygon.hpp"

class SurfaceTriangleObject : public WorldObject {

    TerrainPolygon* m_physics_triangle = nullptr;
    GeometryObject *m_geometry_object = nullptr;
    Material *m_material = nullptr;

public:
    explicit SurfaceTriangleObject(World *world, Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    ~SurfaceTriangleObject() override;

    GeometryObject *get_geometry_object() { return m_geometry_object; }

    void tick(float dt) override;

    void update_position(Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    TerrainPolygon *get_physics_surface() { return m_physics_triangle; }

    void create_colored_mesh(const Vec3f &color);
};