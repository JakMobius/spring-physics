//
// Created by Артем on 31.01.2022.
//

#include "user-controller.hpp"
#include "game-scene.hpp"

UserController::UserController(GameScene* scene)
    : m_scene(scene) {
}

void UserController::on_key_press(Window* window, int key, int scancode, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        window->capture_mouse(false);
    }

    if (key == GLFW_KEY_C) {
        m_scene->toggle_free_camera();
    }

    if (key == GLFW_KEY_R) {
        m_scene->spawn();
    }

    if (key == GLFW_KEY_F3) {
        m_scene->toggle_metrics_window();
    }

    if (m_scene->get_camera_controller())
        m_scene->get_camera_controller()->on_key_press(key, scancode, mods);
}

void UserController::on_key_release(Window* window, int key, int scancode, int mods) {
    if (m_scene->get_camera_controller())
        m_scene->get_camera_controller()->on_key_release(key, scancode, mods);
}

void UserController::on_captured_mouse_move(Window* window, double dx, double dy) {
    if (m_scene->get_camera_controller()) {
        m_scene->get_camera_controller()->on_captured_mouse_move(dx, dy);
    }
}

void UserController::on_mouse_down(Window* window, int button, int mods) {
    window->capture_mouse(true);
}
