#pragma once

#include "../camera/controllers/camera-controller.hpp"

class GameScene;

class UserController : public WindowEventHandler {
    GameScene* m_scene;

  public:
    UserController(GameScene* scene);

    void on_key_press(Window* window, int key, int scancode, int mods) override;
    void on_key_release(Window* window, int key, int scancode, int mods) override;
    void on_captured_mouse_move(Window* window, double dx, double dy) override;
    void on_mouse_down(Window* window, int button, int mods) override;
};