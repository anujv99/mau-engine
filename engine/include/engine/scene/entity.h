#pragma once

#include <entt/entt.hpp>
#include <engine/assert.h>
#include <engine/types.h>

namespace mau {

  class Entity {
  public:
    Entity(entt::entity entity_id, entt::registry& registry) : m_Entity(entity_id), m_Registry(registry) { }
    ~Entity() = default;
  public:
    template <class T, typename... Args>
    T& Add(Args&&... args);

    template <class T>
    T& Get();

    template <class T>
    bool Has() const;

    inline entt::entity GetId() const { return m_Entity; }
  private:
    entt::entity    m_Entity;
    entt::registry& m_Registry;
  };

  template<class T, typename ...Args>
  inline T& Entity::Add(Args&&... args) {
    return m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
  }

  template<class T>
  inline T& Entity::Get() {
    ASSERT(Has<T>());

    return m_Registry.get<T>(m_Entity);
  }

  template<class T>
  inline bool Entity::Has() const {
    return m_Registry.all_of<T>(m_Entity);
  }

}
