
#include "window.hpp"
#include "../imgui/imgui.h"

void WindowEventHandler::on_window_resize(Window* window, int width, int height) {
}

void WindowEventHandler::on_key_press(Window* window, int key, int scancode, int mods) {
}

void WindowEventHandler::on_key_release(Window* window, int key, int scancode, int mods) {
}

void WindowEventHandler::on_mouse_move(Window* window, double x, double y) {
}

void WindowEventHandler::on_mouse_down(Window* window, int button, int mods) {
}

void WindowEventHandler::on_mouse_up(Window* window, int button, int mods) {
}

void WindowEventHandler::on_captured_mouse_move(Window* window, double dx, double dy) {
}

Window::Window(GeneralApp* app)
    : m_app(app) {}
Window::~Window() {
    if (!m_window)
        return;
    glfwDestroyWindow(m_window);
    m_window = nullptr;
}

void Window::initialize(int width, int height, const char* title) {
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, glfw_resize_handler);
    glfwSetKeyCallback(m_window, glfw_key_handler);
    glfwSetMouseButtonCallback(m_window, glfw_mouse_button_handler);
    glfwSetCursorPosCallback(m_window, glfw_mouse_move_handler);
    m_size = {width, height};
}

bool Window::is_opened() {
    return glfwWindowShouldClose(m_window) == 0;
}

void Window::poll_events() {
    glfwPollEvents();
}

Vec2i Window::get_framebuffer_size() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    return {width, height};
}

void Window::glfw_resize_handler(GLFWwindow* glfw_window, int width, int height) {
    auto window = (Window*)glfwGetWindowUserPointer(glfw_window);
    auto handler = window->m_event_handler;
    window->m_size = {width, height};
    if (handler)
        handler->on_window_resize(window, width, height);
}

void Window::glfw_key_handler(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    auto window = (Window*)glfwGetWindowUserPointer(glfw_window);
    auto handler = window->m_event_handler;
    if (action == GLFW_PRESS) {
        handler->on_key_press(window, key, scancode, mods);
    } else if (action == GLFW_RELEASE) {
        handler->on_key_release(window, key, scancode, mods);
    }
}

void Window::glfw_mouse_button_handler(GLFWwindow* glfw_window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    auto window = (Window*)glfwGetWindowUserPointer(glfw_window);
    auto handler = window->m_event_handler;
    if (action == GLFW_PRESS) {
        handler->on_mouse_down(window, button, mods);
    } else if (action == GLFW_RELEASE) {
        handler->on_mouse_up(window, button, mods);
    }
}

void Window::glfw_mouse_move_handler(GLFWwindow* glfw_window, double x, double y) {
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    auto window = (Window*)glfwGetWindowUserPointer(glfw_window);
    auto handler = window->m_event_handler;

    if (window->m_mouse_capture) {
        handler->on_captured_mouse_move(window, x - window->m_mouse_pos.x, y - window->m_mouse_pos.y);
    } else {
        handler->on_mouse_move(window, x, y);
    }
    window->m_mouse_pos = {x, y};
}

void Window::capture_mouse(bool capture) {
    m_mouse_capture = capture;
    if (capture) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
