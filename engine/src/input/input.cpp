#include <engine/input/input.h>

#include <cstring>

namespace mau {

  bool key_state[MAU_KEY_LAST] = {false};
  bool last_key_state[MAU_KEY_LAST] = {false};

  bool mouse_state[MAU_MOUSE_BUTTON_LAST] = {false};
  bool last_mouse_state[MAU_MOUSE_BUTTON_LAST] = {false};

  glm::vec2 mouse_pos = {0.0f, 0.0f};
  glm::vec2 last_mouse_pos = {0.0f, 0.0f};

  glm::vec2 mouse_scroll = {0.0f, 0.0f};

  void Input::OnUpdate() {
    memcpy(last_key_state, key_state, sizeof(key_state));
    memcpy(last_mouse_state, mouse_state, sizeof(mouse_state));
    last_mouse_pos = mouse_pos;
    mouse_scroll = {0.0f, 0.0f};
  }

  void Input::OnEvent(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<KeyPressEvent>(Input::KeyPressEventHandler);
    dispatcher.Dispatch<KeyReleaseEvent>(Input::KeyReleaseEventHandler);
    dispatcher.Dispatch<MousePressEvent>(Input::MousePressEventHandler);
    dispatcher.Dispatch<MouseReleaseEvent>(Input::MouseReleaseEventHandler);
    dispatcher.Dispatch<MouseMoveEvent>(Input::MouseMoveEventHandler);
    dispatcher.Dispatch<MouseScrollEvent>(Input::MouseScrollEventHandler);
  }

  void Input::KeyPressEventHandler(KeyPressEvent &event) { key_state[event.GetKey()] = true; }

  void Input::KeyReleaseEventHandler(KeyReleaseEvent &event) { key_state[event.GetKey()] = false; }

  void Input::MousePressEventHandler(MousePressEvent &event) { mouse_state[event.GetButton()] = true; }

  void Input::MouseReleaseEventHandler(MouseReleaseEvent &event) { mouse_state[event.GetButton()] = false; }

  void Input::MouseMoveEventHandler(MouseMoveEvent &event) { mouse_pos = {event.GetX(), event.GetY()}; }

  void Input::MouseScrollEventHandler(MouseScrollEvent &event) { mouse_scroll = {event.GetX(), event.GetY()}; }

  bool Input::IsKeyDown(TInt32 key) { return key_state[key]; }

  bool Input::IsMouseDown(TInt32 mouse_button) { return mouse_state[mouse_button]; }

  glm::vec2 Input::GetMousePos() { return mouse_pos; }

  glm::vec2 Input::GetMouseOffset() { return mouse_pos - last_mouse_pos; }

  float Input::GetMouseScroll() { return mouse_scroll.y; }

} // namespace mau
