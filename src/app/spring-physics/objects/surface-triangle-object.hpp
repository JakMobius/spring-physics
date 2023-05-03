#pragma once

#include "world-object.hpp"
#include "../physics/vertex.hpp"
#include "../../../world/geometry-object.hpp"
#include "../../../world/shape-generator.hpp"
#include "../physics/surface.hpp"
#include "../physics/terrain-polygon.hpp"

class SurfaceTriangleObject : public WorldObject {

    std::unique_ptr<TerrainPolygon> m_physics_triangle = nullptr;
    std::unique_ptr<GeometryObject> m_geometry_object = nullptr;
    std::unique_ptr<Material> m_material = nullptr;

public:
    explicit SurfaceTriangleObject(World *world, Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    ~SurfaceTriangleObject() override;

    GeometryObject *get_geometry_object() { return m_geometry_object.get(); }

    void tick(float dt) override;

    void update_position(Vec3f vertex_a, Vec3f vertex_b, Vec3f vertex_c);

    TerrainPolygon *get_physics_surface() { return m_physics_triangle.get(); }

    void create_colored_mesh(const Vec3f &color);
};