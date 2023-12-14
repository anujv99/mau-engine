#include <cstring>
#include <engine/window.h>
#include <engine/exceptions.h>
#include <engine/log.h>
#include <engine/input/input.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLFW_CALL(call, reason) {                  \
  int ret = call;                                  \
  if (ret != GLFW_TRUE) {                          \
    throw WindowException(reason);                 \
  }                                                \
}                                                  \

namespace mau {

  // ------------------------- INPUT ------------------------- //

  bool key_state[MAU_KEY_LAST] = { false };
  bool last_key_state[MAU_KEY_LAST] = { false };

  bool mouse_state[MAU_MOUSE_BUTTON_LAST] = { false };
  bool last_mouse_state[MAU_MOUSE_BUTTON_LAST] = { false };

  glm::vec2 mouse_pos = { 0.0f, 0.0f };
  glm::vec2 last_mouse_pos = { 0.0f, 0.0f };

  glm::vec2 mouse_scroll = { 0.0f, 0.0f };
  
  void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("glfw error: %s", description);
  }

  void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // TODO: dispatch events
    key_state[key] = action != GLFW_RELEASE;
  }

  void glfw_mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    // TODO: dispatch events
    mouse_state[button] = action != GLFW_RELEASE;
  }

  void glfw_mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    // TODO: dispatch events
    mouse_pos = { static_cast<float>(xpos), static_cast<float>(ypos) };
  }

  void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // TODO: dispatch events
    mouse_scroll = { static_cast<float>(xoffset), static_cast<float>(yoffset) };
  }

  void update_input() {
    memcpy(last_key_state, key_state, sizeof(key_state));
    memcpy(last_mouse_state, mouse_state, sizeof(mouse_state));
    last_mouse_pos = mouse_pos;
    mouse_scroll = { 0.0f, 0.0f };
  }

  bool Input::IsKeyDown(TInt32 key) {
    return key_state[key];
  }

  bool Input::IsMouseDown(TInt32 mouse_button) {
    return mouse_state[mouse_button];
  }

  glm::vec2 Input::GetMousePos() {
    return mouse_pos;
  }

  glm::vec2 Input::GetMouseOffset() {
    return mouse_pos - last_mouse_pos;
  }

  float Input::GetMouseScroll() {
    return mouse_scroll.y;
  }

  // ------------------------- WINDOW ------------------------- //

  Window::Window(TUint32 width, TUint32 height, std::string_view name) {
    // initialize glfw
    glfwSetErrorCallback(glfw_error_callback);
    GLFW_CALL(glfwInit(), "failed to initialize glfw");

    // create glfw window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name.data(), nullptr, nullptr);

    if (!window) {
      GLFW_CALL(GLFW_FALSE, "failed to create window");
    }

    // set key/mouse callbacks
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_callback);
    glfwSetCursorPosCallback(window, glfw_mouse_move_callback);
    glfwSetScrollCallback(window, glfw_mouse_scroll_callback);

    m_InternalState = reinterpret_cast<void*>(window);
    m_Width = width;
    m_Height = height;

    LOG_INFO("window created [%u, %u]", width, height);
  }

  Window::~Window() {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(m_InternalState);
    if (window) {
      glfwDestroyWindow(window);
      m_InternalState = nullptr;
    }

    glfwTerminate();
  }

  bool Window::ShouldClose() const noexcept {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(m_InternalState);
    if (window) {
      return glfwWindowShouldClose(window) == GLFW_TRUE;
    }

    return true;
  }

  void Window::PollEvents() const noexcept {
    update_input();
    glfwPollEvents();
  }

}
