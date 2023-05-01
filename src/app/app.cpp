
#include "app.hpp"

void App::initialize() {
    GeneralApp::initialize();

    m_window = std::make_unique<Window>(this);
    m_window->initialize(1000, 800, "Vulkan 3D example");

    m_gpu_window = std::make_unique<GpuWindow>(m_window.get());
    m_gpu_window->initialize();

    m_scene = std::make_unique<GameScene>(*m_gpu_window);
    m_window->set_event_handler(m_scene->get_user_controller());

    main_loop();

    m_window->set_event_handler(nullptr);
    m_scene = nullptr;
}

void App::main_loop() {
    while(m_window->is_opened()) {
        m_window->poll_events();
        m_scene->tick();
    }
}
