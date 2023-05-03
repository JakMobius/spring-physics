
#include "gpu-window.hpp"
#include "device-chooser.hpp"

void GpuWindow::create_logical_device() {
    m_physical_device = std::make_unique<VK::PhysicalDevice>(DeviceChooser{this}.choose());

    VK::DeviceFactory device_factory;

    configure_device(device_factory);

    m_device = device_factory.create(m_physical_device.get());
}

bool GpuWindow::is_device_suitable(const VK::PhysicalDevice* device) {
    std::vector<const char*> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    if (!device->supports_extensions(required_extensions))
        return false;

    if (device->get_queue_family_indices()->find_family(VK_QUEUE_GRAPHICS_BIT) < 0)
        return false;
    if (device->get_queue_family_indices()->find_surface_present_family(get_vk_surface()) < 0)
        return false;
    if (!device->has_supported_surface_formats(get_vk_surface()))
        return false;
    if (!device->has_surface_present_modes(get_vk_surface()))
        return false;
    if (!device->get_physical_features()->samplerAnisotropy)
        return false;

    return true;
}
void GpuWindow::create_surface() {
    auto app = m_window->get_app();
    VkSurfaceKHR surface = nullptr;
    if (glfwCreateWindowSurface(app->get_vk_instance().get_handle(), m_window->get_window(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

    m_surface = VK::Surface{&app->get_vk_instance(), surface};
}
void GpuWindow::configure_device(VK::DeviceFactory& device_factory) {
    device_factory.get_enabled_extensions().push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    auto queue_family_indices = m_physical_device->get_queue_family_indices();

    m_graphics_queue_family = queue_family_indices->find_family(VK_QUEUE_GRAPHICS_BIT);
    m_present_queue_family = queue_family_indices->find_surface_present_family(m_surface);

    device_factory.add_queue(0, m_graphics_queue_family, {&m_queue_priority, 1});
    if (m_graphics_queue_family != m_present_queue_family) {
        device_factory.add_queue(0, m_present_queue_family, {&m_queue_priority, 1});
    }

    device_factory.get_validation_layers().push_back("VK_LAYER_KHRONOS_validation");
}
