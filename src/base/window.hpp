#pragma once

#include <etna/vk-wrappers/vk-surface.hpp>
#include "general-app.hpp"
#include "../utils/vec2.hpp"

class Window;

class WindowEventHandler {
public:
    virtual void on_window_resize(Window* window, int width, int height);
    virtual void on_key_press(Window* window, int key, int scancode, int mods);
    virtual void on_key_release(Window* window, int key, int scancode, int mods);
    virtual void on_mouse_move(Window* window, double x, double y);
    virtual void on_mouse_down(Window* window, int button, int mods);
    virtual void on_mouse_up(Window* window, int button, int mods);
    virtual void on_captured_mouse_move(Window* window, double dx, double dy);
};

class Window {
    friend class WindowEventHandler;
public:
    explicit Window(GeneralApp* app);
    virtual ~Window();

    void initialize(int width, int height, const char* title);
    void capture_mouse(bool capture);

    bool is_opened();

    void poll_events();

    Vec2i get_framebuffer_size();

    Vec2i get_size() const { return m_size; }
    GeneralApp* get_app() { return m_app; }
    GLFWwindow* get_window() { return m_window; }

    void set_event_handler(WindowEventHandler* handler) { m_event_handler = handler; }

    bool is_key_pressed(int key) { return glfwGetKey(m_window, key) == GLFW_PRESS; }

private:
    GeneralApp* m_app;
    GLFWwindow* m_window = nullptr;
    Vec2i m_size {};
    Vec2d m_mouse_pos {};
    bool m_destroyed = false;
    bool m_mouse_capture = false;

    WindowEventHandler* m_event_handler = nullptr;

    static void glfw_resize_handler(GLFWwindow* window, int width, int height);
    static void glfw_key_handler(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_mouse_button_handler(GLFWwindow* window, int button, int action, int mods);
    static void glfw_mouse_move_handler(GLFWwindow* window, double x, double y);
};