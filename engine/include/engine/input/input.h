#pragma once

#include <engine/types.h>
#include <engine/input/keycodes.h>
#include <engine/input/mousecodes.h>
#include <glm/glm.hpp>

namespace mau {
  class Input {
  public:
    static bool IsKeyDown(TInt32 key);
    static bool IsMouseDown(TInt32 mouse_button);

    static glm::vec2 GetMousePos();
    static glm::vec2 GetMouseOffset();
    static float GetMouseScroll();
  };
}

