#include <engine/scene/scene.h>
#include <engine/assert.h>

namespace mau {

  Scene::Scene() { }

  Scene::~Scene() { m_Registry.clear(); }

  Entity Scene::CreateEntity(const String &name) {
    entt::entity entity_id = m_Registry.create();

    Entity entity(entity_id, m_Registry);
    entity.Add<TransformComponent>();
    entity.Add<NameComponent>(name.empty() ? std::to_string(static_cast<ENTT_ID_TYPE>(entity_id)) : name);

    return entity;
  }

  Entity Scene::GetEntity(entt::entity entity_id) {
    ASSERT(m_Registry.valid(entity_id));
    return Entity(entity_id, m_Registry);
  }

  void Scene::Each(EntityIterator func) {
    for (auto [entity_id] : m_Registry.storage<entt::entity>().each()) {
      Entity entity(entity_id, m_Registry);
      func(entity);
    }
  }

} // namespace mau
