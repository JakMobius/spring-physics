#pragma once

class World;

class WorldObject {
  protected:
    World* m_world;

  public:
    WorldObject(World* world)
        : m_world(world) {}
    virtual ~WorldObject() {}

    virtual void tick(float dt) {}

    World* get_world() {
        return m_world;
    }
};