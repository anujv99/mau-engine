#pragma once

#include <glm/glm.hpp>

namespace mau {

  class Camera {
  public:
    Camera() = default;
    ~Camera() = default;

  public:
    glm::mat4 GetView() const;
    glm::mat4 GetProj(glm::vec2 window_size) const;
    glm::mat4 GetMVP(glm::vec2 window_size) const;

  public:
    glm::vec3 Position = glm::vec3(0.0f);
    glm::vec3 Direction = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
  };

} // namespace mau
