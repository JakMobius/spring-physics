#pragma once

#include "../../../base/window.hpp"
#include "../../../utils/vec3.hpp"
#include "../camera.hpp"
#include "camera-controller.hpp"

class FreeCameraController : public CameraController {
public:
    explicit FreeCameraController(Window *window) : CameraController(window) {};

    void on_tick(float delta_time) override;

    Vec3f get_movement();

    void on_key_press(int key, int scancode, int mods) override;
    void on_key_release(int key, int scancode, int mods) override;

    void on_captured_mouse_move(double dx, double dy) override;

private:
    float m_speed = 0.1;
    float m_rotation_speed = 0.1;
    float m_mouse_sensitive = 0.001f;

    float m_pitch = 0;
    float m_camera_speed = 10.0f;
    float yaw = 0;
};