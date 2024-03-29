
#include "shape-generator.hpp"
#include "shapes/icosahedron.hpp"

void ShapeGenerator::add_triangle(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, Material* material) {
    Vec3f a = p3 - p1;
    Vec3f b = p2 - p1;
    Vec3f normal = a.cross(b);
    normal.normalize();
    add_vertex(p1, normal, {}, material);
    add_vertex(p2, normal, {}, material);
    add_vertex(p3, normal, {}, material);
}

void ShapeGenerator::add_square(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3, const Vec3f& p4,
                                Material* material) {
    add_triangle(p1, p4, p2, material);
    add_triangle(p1, p3, p4, material);
}

void ShapeGenerator::add_cube(const Vec3f& center, const Vec3f& size, Material* material) {
    Material* materials[] = {material, material, material, material, material, material};
    add_cube(center, size, materials);
}

void ShapeGenerator::add_cube(const Vec3f& center, const Vec3f& size, Material** materials) {

    Vec3f masks[3] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}};
    for (int i = 0; i < 3; i++) {

        Vec3f basis_u = masks[(i + 1) % 3] * size;
        Vec3f basis_v = masks[(i + 2) % 3] * size;

        Vec3f offset = center + masks[i] * size * 0.5 - (basis_u + basis_v) * 0.5;

        add_square(offset,
                   offset + basis_u,
                   offset + basis_v,
                   offset + basis_u + basis_v, materials[i]);

        offset -= masks[i] * size;

        add_square(offset,
                   offset + basis_v,
                   offset + basis_u,
                   offset + basis_u + basis_v, materials[i + 3]);
    }
}

void ShapeGenerator::add_vertex(const Vec3f& position, const Vec3f& normal, const Vec2f& uv, Material* material) {
    m_mesh.emplace_back(position, normal, uv, material);
}

void ShapeGenerator::reset() {
    m_mesh.clear();
}

void ShapeGenerator::add_sphere(const Vec3f& center, float radius, Material* material, int lod) {
    std::vector<Vec3f> sphere{};

    for (auto& face : IcosahedronShape::faces) {
        Vec3f vertices[3] = {
            IcosahedronShape::vertices[face[0]],
            IcosahedronShape::vertices[face[1]],
            IcosahedronShape::vertices[face[2]],
        };

        for (auto& vertex : vertices) {
            vertex /= IcosahedronShape::icosahedron_radius;
            sphere.push_back(vertex);
        }
    }

    for (int i = 0; i < lod; i++) {
        increase_sphere_details(sphere);
    }

    for (auto& vertex : sphere) {
        add_vertex(vertex* radius += center, vertex, {}, material);
    }
}
