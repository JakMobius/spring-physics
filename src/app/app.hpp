#pragma once

class App;

#include "../base/general-app.hpp"
#include "../base/gpu-window.hpp"
#include "../base/window.hpp"
#include "spring-physics/game-scene.hpp"

class App : public GeneralApp {
    std::unique_ptr<Window> m_window {};
    std::unique_ptr<GpuWindow> m_gpu_window {};
    std::unique_ptr<GameScene> m_scene {};
public:
    App() = default;

    void configure(VK::InstanceFactory &factory) override {
        GeneralApp::configure(factory);
        factory.get_enabled_layer_names().push_back("VK_LAYER_KHRONOS_validation");
    }

    void initialize() override;

    void main_loop();
};