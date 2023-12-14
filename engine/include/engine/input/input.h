#pragma once

#include <engine/types.h>
#include <engine/events/event.h>
#include <engine/events/key-events.h>
#include <engine/events/mouse-events.h>
#include <engine/input/keycodes.h>
#include <engine/input/mousecodes.h>
#include <glm/glm.hpp>

namespace mau {
  class Input {
  public:
    static void OnUpdate();
    static void OnEvent(Event& event);

    static bool IsKeyDown(TInt32 key);
    static bool IsMouseDown(TInt32 mouse_button);

    static glm::vec2 GetMousePos();
    static glm::vec2 GetMouseOffset();
    static float GetMouseScroll();
  private:
    static void KeyPressEventHandler(KeyPressEvent& event);
    static void KeyReleaseEventHandler(KeyReleaseEvent& event);
    static void MousePressEventHandler(MousePressEvent& event);
    static void MouseReleaseEventHandler(MouseReleaseEvent& event);
    static void MouseMoveEventHandler(MouseMoveEvent& event);
    static void MouseScrollEventHandler(MouseScrollEvent& event);
  };
}

