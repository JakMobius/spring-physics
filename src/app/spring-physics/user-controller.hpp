#pragma once


#include "../camera/controllers/camera-controller.hpp"

class GameScene;

class UserController : public WindowEventHandler {
    GameScene* m_scene;
    CameraController* m_camera_controller = nullptr;

public:

    UserController(GameScene* scene);

    void on_tick(float dt);

    void on_key_press(Window *window, int key, int scancode, int mods) override;
    void on_key_release(Window *window, int key, int scancode, int mods) override;
    void on_captured_mouse_move(Window *window, double dx, double dy) override;
    void on_mouse_down(Window *window, int button, int mods) override;

    void set_camera_controller(CameraController* camera_controller);
};