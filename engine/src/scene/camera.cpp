#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mau {

  glm::mat4 Camera::GetMVP(glm::vec2 window_size) const {
    const glm::mat4 projection = glm::perspective(glm::radians(80.0f), window_size.x / window_size.y, 0.1f, 10000.0f);
    const glm::mat4 view = glm::lookAt(Position, Position + Direction, Up);

    return projection * view;
  }

}
