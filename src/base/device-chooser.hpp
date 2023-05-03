#pragma once

#include "gpu-window.hpp"
#include "window.hpp"
#include <etna/vk-wrappers/device.hpp>

class DeviceChooser {
    GpuWindow* m_gpu_window = nullptr;

  public:
    DeviceChooser(GpuWindow* window)
        : m_gpu_window(window) {
    }

    VK::PhysicalDevice choose() {

        auto app = m_gpu_window->get_window()->get_app();
        const VK::PhysicalDevice* any_suitable_gpu = nullptr;

        for (auto& device : app->get_vk_instance().get_physical_devices()) {
            if (!is_device_suitable(&device))
                continue;

            if (device.get_physical_properties()->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return device;
            }

            any_suitable_gpu = &device;
        }

        if (!any_suitable_gpu) {
            throw std::runtime_error("could not find any suitable GPU");
        }

        return *any_suitable_gpu;
    }

    bool is_device_suitable(const VK::PhysicalDevice* device) {
        if (!m_gpu_window->is_device_suitable(device))
            return false;

        return true;
    }
};
