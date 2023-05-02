#pragma once

#include "window.hpp"
#include <etna/basic-attachment-manager.hpp>
#include <etna/command-queue/command-queue-pool.hpp>
#include <etna/vk-wrappers/command-buffer/vk-command-pool.hpp>
#include <etna/vk-wrappers/device/vk-device-factory.hpp>

class GpuWindow {
protected:
    Window* m_window;
    VK::Surface m_surface {};
    VK::Device m_device {};
    std::unique_ptr<VK::PhysicalDevice> m_physical_device {};

    void create_surface();

    void create_logical_device();

protected:

    virtual void configure_device(VK::DeviceFactory& device_factory);

public:
    GpuWindow(Window* window): m_window(window) {

    }

    virtual ~GpuWindow() { }

    virtual void initialize() {
        create_surface();
        create_logical_device();

        m_device_graphics_queue = m_device.get_queue(m_graphics_queue_family, 0);
        m_device_present_queue = m_device.get_queue(m_present_queue_family, 0);
    }

    int get_graphics_queue_family() const { return m_graphics_queue_family; }
    int get_present_queue_family() const { return m_present_queue_family; }

    VK::Queue get_device_graphics_queue() const { return m_device_graphics_queue; }
    VK::Queue get_device_present_queue() const { return m_device_present_queue; }

    bool is_device_suitable(const VK::PhysicalDevice *device);

    int m_graphics_queue_family = -1;
    int m_present_queue_family = -1;
    float m_queue_priority = 1.0f;

    VK::Queue m_device_graphics_queue {};
    VK::Queue m_device_present_queue {};

    VK::Device& get_device() { return m_device; };
    VK::Surface& get_vk_surface() { return m_surface; }
    Window* get_window() { return m_window; }
};