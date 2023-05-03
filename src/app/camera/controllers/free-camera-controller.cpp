
#include "free-camera-controller.hpp"
#include <GLFW/glfw3.h>

void FreeCameraController::on_tick(float delta_time) {
    Vec3f camera_space_speed = get_movement() * m_camera_speed * delta_time;
    Vec3f camera_speed = {0, 0, 0};

    Vec3f camera_front_direction = m_controlled_camera->get_direction();
    camera_front_direction.set(1, 0);
    camera_front_direction.normalize();

    Vec3f camera_right_direction = Vec3f{camera_front_direction.z, 0, -camera_front_direction.x};

    camera_speed += camera_front_direction * camera_space_speed.z;
    camera_speed += camera_right_direction * camera_space_speed.x;
    camera_speed += Vec3f{0, 1, 0} * camera_space_speed.y;

    m_controlled_camera->set_position(camera_speed + m_controlled_camera->get_position());
}

Vec3f FreeCameraController::get_movement() {
    Vec3f result;
    if (m_window->is_key_pressed(GLFW_KEY_W))
        result.z = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_A))
        result.x = -1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_S))
        result.z = -1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_D))
        result.x = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_SPACE))
        result.y = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_LEFT_SHIFT))
        result.y = -1.0f;
    return result;
}

void FreeCameraController::on_captured_mouse_move(double dx, double dy) {
    m_pitch += dy * m_mouse_sensitive;
    yaw -= dx * m_mouse_sensitive;
    m_controlled_camera->set_pitch_yaw(m_pitch, yaw);
}

void FreeCameraController::on_key_press(int key, int scancode, int mods) {
    if (key == GLFW_KEY_LEFT_CONTROL) {
        m_camera_speed = 50;
    }
}

void FreeCameraController::on_key_release(int key, int scancode, int mods) {
    if (key == GLFW_KEY_LEFT_CONTROL) {
        m_camera_speed = 10;
    }
}
