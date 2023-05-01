
#include "game-scene.hpp"
#include "../camera/controllers/follow-creature-camera.hpp"
#include "../camera/controllers/free-camera-controller.hpp"
#include "models/airplane-creature.hpp"
#include "../../utils/get-orthogonal.hpp"

void GameScene::update_inputs() {
    m_throttle = 0.0f;
    m_input_airplane_controls = Vec3f(0, 0, 0);
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_T)) m_throttle = 1.0f;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_UP)) m_input_airplane_controls.y = -1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_DOWN)) m_input_airplane_controls.y = 1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_LEFT)) m_input_airplane_controls.x = -1;
    if (m_window.get_window()->is_key_pressed(GLFW_KEY_RIGHT)) m_input_airplane_controls.x = 1;
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
    m_user_controller.set_camera_controller(m_camera_controller.get());
}

void GameScene::build_model(const Matrix4f& transform) {
    m_creature = std::make_unique<AirplaneCreature>(m_world.get(), transform);
    //    m_creature = std::make_unique<CarCreature>(m_world.get(), transform);
    //    m_creature = std::make_unique<NWheelerCreature>(m_world.get(), transform);
    //    m_creature = std::make_unique<TrainCreature>(m_world.get(), transform);
}

GameScene::GameScene(GpuWindow &window) : m_window(window) {
    m_drawer = std::make_unique<SceneRenderer>(window);
    m_drawer->initialize();

    m_world = std::make_unique<World>(m_drawer->get_rendering_context());
    m_camera = std::make_unique<PerspectiveCamera>();
    m_shadow_camera = std::make_unique<OrthogonalCamera>();
    m_camera->set_position({0, 3, -3});

//    m_world->get_drawer()->add_light({{0.3, -0.8, 0.5},
//                                        {0.6, 0.6,  0.6}});
    m_map_object = std::make_unique<MapObject>(m_world.get(), "resources/maps/map2.obj", Matrix4f::scale_matrix(0.03, 0.03, 0.03) * Matrix4f::translation_matrix(0, 5, 0));
    // On the border (for car & physics testing)
    build_model({});

    //    m_runway_texture = m_world->get_renderer()->get_texture_manager()->load_texture("resources/textures/runway.png");
    //    m_runway_texture->make_resident(true);

    toggle_free_camera();
}

void GameScene::tick() {
    update_inputs();

    float dt = 1.0f / 60.0f;

    m_user_controller.on_tick(dt);

    m_real_airplane_controls = m_input_airplane_controls * 0.1 + m_real_airplane_controls * 0.9;

    if(m_creature) {
        m_creature->set_controls(m_real_airplane_controls);
        m_creature->set_throttle(m_throttle);
    }

    auto size = m_window.get_window()->get_size();
    m_camera->set_aspect((float) size.x / (float) size.y);
    m_world->get_rendering_context()->m_camera = m_camera.get();

    m_world->get_physics_engine()->tick();
    m_world->tick(dt);

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

    m_drawer->draw();
}
