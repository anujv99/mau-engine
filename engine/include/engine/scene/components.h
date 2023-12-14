#pragma once

#include <engine/types.h>
#include <glm/glm.hpp>

namespace mau {

  struct TransformComponent {
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Rotation = glm::vec3(0.0f);
    glm::vec3 Scale    = glm::vec3(1.0f);
  };

  struct NameComponent {
    String Name = "";
  };

}