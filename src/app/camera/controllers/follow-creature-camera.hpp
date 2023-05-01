#pragma once

#include "camera-controller.hpp"
#include "../../spring-physics/controllable-creature.hpp"
#include "../../spring-physics/creatures/creature.hpp"
#include "../camera.hpp"

class FollowCreatureCamera : public CameraController {
public:
    explicit FollowCreatureCamera(Window* window, ControllableCreature *creature): CameraController(window), m_creature(creature) {};

    void on_tick(float delta_time) override;
    Vec3f get_movement();

    void on_captured_mouse_move(double dx, double dy) override;

private:
    ControllableCreature *m_creature;
    float m_speed = 4.0;
    float m_rotation_speed = 2.0;

    float m_view_angle = 0.0f;
    Vec3f m_offset = Vec3f(0.0f, 1.0f, 0.0f);
    float m_camera_distance = 10.0f;
    float m_mouse_sensitive = 0.001f;

    bool old_speed_low = false;
    float old_camera_heading = 0.0f;
    Vec3f m_old_creature_position = Vec3f(0.0f, 0.0f, 0.0f);
};