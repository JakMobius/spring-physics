
#include "game-scene.hpp"
#include "../../utils/get-orthogonal.hpp"
#include "../camera/controllers/follow-creature-camera.hpp"
#include "../camera/controllers/free-camera-controller.hpp"
#include "models/airplane-creature.hpp"
#include "models/car/car-creature.hpp"
#include "../../imgui/imgui_internal.h"

void GameScene::update_inputs() {
    m_throttle = 0.0f;
    m_input_airplane_controls = Vec3f(0, 0, 0);
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_T))
        m_throttle = 1.0f;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_UP))
        m_input_airplane_controls.y = -1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_DOWN))
        m_input_airplane_controls.y = 1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_LEFT))
        m_input_airplane_controls.x = -1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_RIGHT))
        m_input_airplane_controls.x = 1;

    if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        int count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

        if (count >= 6) {
            m_throttle += (axes[4] + 1) / 2;
            m_gamepad_airplane_controls.set(0, axes[0]);
            m_gamepad_airplane_controls.set(1, axes[1]);
        }
    } else {
        m_gamepad_airplane_controls = Vec3f(0, 0, 0);
    }
}

void GameScene::toggle_free_camera() {
    m_free_camera = !m_free_camera;
    if (m_free_camera || !m_creature) {
        m_camera_controller = std::make_unique<FreeCameraController>(m_window.get_window());
        m_free_camera = true;
    } else {
        m_camera_controller = std::make_unique<FollowCreatureCamera>(m_window.get_window(), m_creature.get());
    }
    m_camera_controller->set_controlled_camera(m_camera.get());
}

void GameScene::build_model(const Matrix4f& transform) {
//    m_creature = std::make_unique<AirplaneCreature>(m_world.get(), transform);
        m_creature = std::make_unique<CarCreature>(m_world.get(), transform);
    //    m_creature = std::make_unique<NWheelerCreature>(m_world.get(), transform);
    //    m_creature = std::make_unique<TrainCreature>(m_world.get(), transform);
}

GameScene::GameScene(GpuWindow& window)
    : m_window(window) {
    m_drawer = std::make_unique<SceneRenderer>(window);
    m_drawer->initialize();

    m_world = std::make_unique<World>(m_drawer->get_rendering_context());
    m_camera = std::make_unique<PerspectiveCamera>();
    m_shadow_camera = std::make_unique<OrthogonalCamera>();
    m_camera->set_position({0, 3, -3});

    //    m_world->get_drawer()->add_light({{0.3, -0.8, 0.5},
    //                                        {0.6, 0.6,  0.6}});
//    m_map_object = std::make_unique<MapObject>(m_world.get(), "resources/maps/map2.obj", Matrix4f::scale_matrix(0.03, 0.03, 0.03) * Matrix4f::translation_matrix(0, 5, 0));
    m_map_object = std::make_unique<ProceduralWorldObject>(m_world.get());

    spawn();
    //    m_runway_texture = m_world->get_renderer()->get_texture_manager()->load_texture("resources/textures/runway.png");
    //    m_runway_texture->make_resident(true);

    toggle_free_camera();
}

void GameScene::tick() {
    update_inputs();

    float dt = 1.0f / 60.0f;

    if (m_camera_controller)
        m_camera_controller->on_tick(dt);

    Vec3f controls = m_input_airplane_controls + m_gamepad_airplane_controls;

    m_real_airplane_controls = controls * 0.1 + m_real_airplane_controls * 0.9;

    Vec3f old_creature_position = m_creature->get_creature()->get_center();

    if (m_creature) {
        m_creature->set_controls(m_real_airplane_controls);
        m_creature->set_throttle(m_throttle);
    }

    auto size = m_window.get_window()->get_size();
    m_camera->set_aspect((float)size.x / (float)size.y);
    m_world->get_rendering_context()->m_camera = m_camera.get();

    m_world->get_physics_engine()->tick();
    m_world->tick(dt);

    m_map_object->begin_far_chunk_sweep();
    for(auto& vertex : m_creature->get_creature()->get_vertices()) {
        m_map_object->mark_creature_position(vertex->get_physics_vertex()->m_position);
    }
    m_map_object->account_for_camera(*m_camera);
    m_map_object->commit_far_chunk_sweep();
    m_map_object->update_chunk_physics();

    m_creature_speed = (m_creature->get_creature()->get_center() - old_creature_position).len() / dt;

    auto camera_position = m_camera->get_position();

    auto sun_direction = m_world->get_rendering_context()->m_sun_direction;

    auto orthogonal_camera_position = camera_position - sun_direction * 1000.0f;

    m_shadow_camera->set_far(10000.0f);
    m_shadow_camera->set_near(0.1f);
    m_shadow_camera->set_lb_corner({-40, -40});
    m_shadow_camera->set_rt_corner({40, 40});
    m_shadow_camera->set_position(orthogonal_camera_position);
    m_shadow_camera->set_direction(sun_direction, get_orthogonal_vector(sun_direction));

    m_world->get_rendering_context()->m_shadow_mapping_matrix = m_shadow_camera->get_matrix();

    //    m_world->get_drawer()->m_camera_matrix = m_shadow_camera->get_matrix();
    //    m_world->get_drawer()->m_camera_position = orthogonal_camera_position;

    m_drawer->prepare_for_frame();

    if (m_show_metrics_window) {
        ImGuiContext& ctx = *ImGui::GetCurrentContext();
        ImGuiIO& io = ctx.IO;

        if (!ImGui::Begin("Metrics")) {
            ImGui::End();
        } else {
            auto& geometry_pool = m_drawer->get_rendering_context()->m_geometry_pool;

            ImGui::SeparatorText("Graphics");
            ImGui::Text("FPS: %.1f, %.3f ms/frame", io.Framerate, 1000.0f / io.Framerate);
            ImGui::Text("Vertex buffer fragments: %zu", geometry_pool->get_vertex_multirange().get_intervals().size());
            ImGui::Text("Vertex buffer size: ");
            print_buffer_size(geometry_pool->get_vertex_buffer()->get_storage().size() * sizeof(float));
            ImGui::Text("Material buffer size: ");
            print_buffer_size(geometry_pool->get_material_buffer()->get_storage().size() * sizeof(float));
            ImGui::Text("Transform buffer size: ");
            print_buffer_size(geometry_pool->get_matrix_buffer()->get_storage().size() * sizeof(float));
            ImGui::Text("Total scene vertices: %zu", geometry_pool->get_vertex_multirange().size());
            ImGui::Text("Chunks loaded: %zu", m_map_object->get_chunks().size());

            if(m_creature) {
                ImGui::SeparatorText("Creature");
                auto creature = m_creature->get_creature();
                auto position = creature->get_center();
                ImGui::Text("Creature vertices: %zu", creature->get_vertices().size());
                ImGui::Text("Creature surfaces: %zu", creature->get_surfaces().size());
                ImGui::Text("Creature springs: %zu", creature->get_springs().size());
                ImGui::Text("Creature center: (%.1f, %.1f, %.1f)", position.x, position.y, position.z);
            }



            ImGui::End();
        }
    }

    show_speed_overlay();
    //
    //    ImGui::Begin("Colors");
    //    ImGui::ColorEdit3("Ambient color", (float*)&m_rendering_context.m_ambient_color);
    //    ImGui::ColorEdit3("Clear color", (float*)&m_rendering_context.m_clear_color);
    //    ImGui::ColorEdit3("Fog color", (float*)&m_rendering_context.m_fog_color);
    //    ImGui::SliderFloat("Fog amount", (float*)&m_rendering_context.m_fog_amount, 0, 0.1);
    //
    //    ImGui::End();

    m_drawer->draw_frame();
}

void GameScene::spawn() {
    if (!m_free_camera) {
        m_camera_controller = nullptr;
    }
    // On the border (for car & physics testing)
    build_model(Matrix4f::translation_matrix(0, 3, 0));
    if (!m_free_camera) {
        m_camera_controller = std::make_unique<FollowCreatureCamera>(m_window.get_window(), m_creature.get());
        m_camera_controller->set_controlled_camera(m_camera.get());
    }
}
void GameScene::show_speed_overlay() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowBgAlpha(0.35f);

    auto size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(size.x - 10.0f, size.y - 10.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::Begin("Speed", nullptr, window_flags);
    ImGui::Text("%.2f m/s", m_creature_speed);
    ImGui::End();
}
