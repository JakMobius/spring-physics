#pragma once

#include "../../../base/window.hpp"
#include "../camera.hpp"

class CameraController {

protected:
    Window* m_window;
    Camera* m_controlled_camera;

public:
    CameraController(Window* window) : m_window(window) {}

    void set_controlled_camera(Camera* camera) {
        m_controlled_camera = camera;
    }

    virtual void on_key_press(int key, int scancode, int mods) {}
    virtual void on_key_release(int key, int scancode, int mods) {}

    virtual void on_captured_mouse_move(double dx, double dy) {}

    virtual void on_tick(float delta_time) {}
};