
#include "follow-creature-camera.hpp"

void FollowCreatureCamera::on_tick(float delta_time) {
    Vec3f creature_position = m_creature->get_camera_binding_point();

    Vec3f delta = creature_position - m_old_creature_position;
    m_old_creature_position = creature_position;

    delta.set(1, 0);

    float camera_angle = old_camera_heading;

    if (delta.len() > 0.1) {
        float creature_heading = 0;
        creature_heading = -atan2(delta.x, delta.z);
        float desired_camera_angle = creature_heading + M_PI / 2;
        if (old_speed_low) {
            m_view_angle += old_camera_heading - desired_camera_angle;
        }
        camera_angle = desired_camera_angle;
        old_speed_low = false;
    } else {
        old_speed_low = true;
    }

    old_camera_heading = camera_angle;

    camera_angle += m_view_angle;

    Vec3f m_angle_vector = Vec3f(sin(camera_angle), 0.0f, -cos(camera_angle));

    Vec3f camera_position = creature_position + m_angle_vector * m_camera_distance + m_offset;

    m_controlled_camera->set_position(camera_position);

    float pitch = atan2(m_offset.y, m_camera_distance);

    m_controlled_camera->set_pitch_yaw(pitch, camera_angle);

    auto camera_speed = get_movement();
    m_view_angle += camera_speed.x * delta_time * m_rotation_speed;
    m_camera_distance += camera_speed.z * delta_time * m_speed;
    m_offset.y += camera_speed.y * delta_time * m_speed;
}

Vec3f FollowCreatureCamera::get_movement() {
    Vec3f result = Vec3f(0.0f, 0.0f, 0.0f);
    if (m_window->is_key_pressed(GLFW_KEY_W))
        result.z = -1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_S))
        result.z = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_A))
        result.x = -1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_D))
        result.x = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_SPACE))
        result.y = 1.0f;
    if (m_window->is_key_pressed(GLFW_KEY_LEFT_SHIFT))
        result.y = -1.0f;

    if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        int count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

        if (count >= 6) {
            Vec3f joystick_controls = Vec3f(axes[2], -axes[5], 0.0f);
            float len = joystick_controls.len();
            if (len > 0.05f) {
                joystick_controls -= joystick_controls * 0.05f / len;
                result += joystick_controls;
            }
        }
    }
    return result;
}

void FollowCreatureCamera::on_captured_mouse_move(double dx, double dy) {
    CameraController::on_captured_mouse_move(dx, dy);

    m_view_angle -= dx * m_mouse_sensitive * m_rotation_speed;
    m_offset.y += dy * m_mouse_sensitive * m_speed;
}
