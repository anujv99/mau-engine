#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mau {

  glm::mat4 Camera::GetView() const {
    return glm::lookAt(Position, Position + Direction, Up);
  }

  glm::mat4 Camera::GetProj(glm::vec2 window_size) const {
    return glm::perspective(glm::radians(80.0f), window_size.x / window_size.y,
                            0.001f, 10000.0f);
  }

  glm::mat4 Camera::GetMVP(glm::vec2 window_size) const {
    const glm::mat4 projection = GetProj(window_size);
    const glm::mat4 view = GetView();

    return projection * view;
  }

} // namespace mau
