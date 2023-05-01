//
// Created by Артем on 31.01.2022.
//

#include "user-controller.hpp"
#include "game-scene.hpp"

UserController::UserController(GameScene *scene) : m_scene(scene) {

}

void UserController::on_tick(float dt) {
    m_camera_controller->on_tick(dt);
}

void UserController::set_camera_controller(CameraController *camera_controller) {
    m_camera_controller = camera_controller;
}

void UserController::on_key_press(Window* window, int key, int scancode, int mods) {
    if(key == GLFW_KEY_ESCAPE) {
        window->capture_mouse(false);
    }

    if(key == GLFW_KEY_C) {
        m_scene->toggle_free_camera();
    }

    if(m_camera_controller)
        m_camera_controller->on_key_press(key, scancode, mods);
}

void UserController::on_key_release(Window* window, int key, int scancode, int mods) {
    if (m_camera_controller)
        m_camera_controller->on_key_release(key, scancode, mods);
}

void UserController::on_captured_mouse_move(Window* window, double dx, double dy) {
    if (m_camera_controller) {
        m_camera_controller->on_captured_mouse_move(dx, dy);
    }
}

void UserController::on_mouse_down(Window* window, int button, int mods) {
    window->capture_mouse(true);
}
