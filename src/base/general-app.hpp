#pragma once

#include <iostream>
#include <GLFW/glfw3.h>
#include <etna/vk-wrappers/instance.hpp>
#include <etna/vk-wrappers/vk-debug-callback-handler.hpp>

class GeneralApp {

  protected:
    VK::Instance m_instance{};
    VK::DebugCallbackHandler m_debug_callback_handler{};

  public:
    GeneralApp() = default;
    GeneralApp(const GeneralApp& copy) = delete;
    GeneralApp(GeneralApp&& move) = delete;
    ~GeneralApp() {
        glfwTerminate();
    }

    virtual void configure(VK::InstanceFactory& factory) {
        factory.set_app_name("Unnamed app");
        factory.set_app_version({1, 0, 0});
        ;
        factory.get_enabled_extension_names() = get_required_instance_extensions();
    }

    virtual void initialize() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        VK::InstanceFactory factory{};
        configure(factory);
        m_instance = factory.create();

        if (!m_debug_callback_handler.listen(m_instance.get_handle())) {
            throw std::runtime_error("debug callback handler failed to initialize");
        }
    };

    std::vector<const char*> get_required_instance_extensions() const {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions = nullptr;

        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    VK::Instance& get_vk_instance() {
        return m_instance;
    }
    VK::DebugCallbackHandler& get_debug_callback_handler() {
        return m_debug_callback_handler;
    }

    const VK::Instance& get_vk_instance() const {
        return m_instance;
    }
    const VK::DebugCallbackHandler& get_debug_callback_handler() const {
        return m_debug_callback_handler;
    }
};