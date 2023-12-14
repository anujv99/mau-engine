#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <engine/types.h>
#include <engine/utils/handle.h>

#include <engine/scene/entity.h>
#include <engine/scene/components.h>

namespace mau {

  using EntityIterator = std::function<void(Entity entity)>;

  class Scene: public HandledObject {
  public:
    Scene();
    ~Scene();
  public:
    Entity CreateEntity(const String& name = "");
    Entity GetEntity(entt::entity entity_id);

    void Each(EntityIterator func);
  private:
    entt::registry m_Registry;
  };

}
