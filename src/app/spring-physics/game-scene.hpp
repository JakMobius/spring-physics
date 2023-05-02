
#include "../camera/orthogonal_camera.hpp"
#include "../camera/perspective_camera.hpp"
#include "../renderer/scene-renderer.hpp"
#include "controllable-creature.hpp"
#include "objects/map-object.hpp"
#include "user-controller.hpp"
#include "world.hpp"

class GameScene {

    UserController m_user_controller { this };

    GpuWindow& m_window;

    std::unique_ptr<PerspectiveCamera> m_camera {};
    std::unique_ptr<OrthogonalCamera> m_shadow_camera {};
    std::unique_ptr<CameraController> m_camera_controller {};
    std::unique_ptr<SceneRenderer> m_drawer {};
    std::unique_ptr<World> m_world {};
    std::unique_ptr<MapObject> m_map_object {};

    GeometryObject* object = nullptr;

    Vec3f m_input_airplane_controls {};
    Vec3f m_real_airplane_controls {};

    std::unique_ptr<ControllableCreature> m_creature {};

    bool m_free_camera = false;
    float m_throttle = 0.0f;

    void update_inputs();

public:

    GameScene(GpuWindow& window);
    ~GameScene() {
      if(m_drawer) {
        m_drawer->deinitialize();
      }
    }

    void tick();
    void on_draw();

    UserController* get_user_controller() { return &m_user_controller; }

    void build_model(const Matrix4f& transform);
    void toggle_free_camera();
};