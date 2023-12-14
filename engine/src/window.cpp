#include <cstring>
#include <engine/window.h>
#include <engine/exceptions.h>
#include <engine/log.h>
#include <engine/input/input.h>
#include <engine/events/key-events.h>
#include <engine/events/mouse-events.h>
#include <engine/events/window-events.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLFW_CALL(call, reason) {                  \
  int ret = call;                                  \
  if (ret != GLFW_TRUE) {                          \
    throw WindowException(reason);                 \
  }                                                \
}                                                  \

namespace mau {

  void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("glfw error: %s", description);
  }

  EventEmitFunction get_event_emit_func(GLFWwindow* window) {
    Window* user_pointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (!user_pointer) {
      LOG_ERROR("failed to get window user pointer");
      return nullptr;
    }

    EventEmitFunction func = user_pointer->GetEventCallback();
    if (!func) {
      LOG_ERROR("failed to get event callback");
      return nullptr;
    }

    return func;
  }

  void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;
    switch (action) {
      case GLFW_PRESS: {
        KeyPressEvent event(key);
        func(event);
        break;
      }
      case GLFW_RELEASE: {
        KeyReleaseEvent event(key);
        func(event);
        break;
      }
      case GLFW_REPEAT: {
        KeyRepeatEvent event(key);
        func(event);
        break;
      }
    }
  }

  void glfw_mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;

    switch (action) {
      case GLFW_PRESS: {
        MousePressEvent event(button);
        func(event);
        break;
      }
      case GLFW_RELEASE: {
        MouseReleaseEvent event(button);
        func(event);
        break;
      }
    }
  }

  void glfw_mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;

    MouseMoveEvent event(static_cast<float>(xpos), static_cast<float>(ypos));
    func(event);
  }

  void glfw_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;

    MouseScrollEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
    func(event);
  }

  void glfw_window_close_callback(GLFWwindow* window) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;

    WindowCloseEvent event;
    func(event);
  }

  void glfw_window_resize_callback(GLFWwindow* window, int width, int height) {
    EventEmitFunction func = get_event_emit_func(window);
    if (!func) return;

    WindowResizeEvent event(static_cast<TUint32>(width), static_cast<TUint32>(height));
    func(event);
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

    glfwSetWindowUserPointer(window, this);
    // set key/mouse callbacks
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_callback);
    glfwSetCursorPosCallback(window, glfw_mouse_move_callback);
    glfwSetScrollCallback(window, glfw_mouse_scroll_callback);
    glfwSetWindowCloseCallback(window, glfw_window_close_callback);
    glfwSetWindowSizeCallback(window, glfw_window_resize_callback);

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
    glfwPollEvents();
  }

  void Window::RegisterEventCallback(std::function<void(Event&)> callback) noexcept {
    m_EventCallback = callback;
  }
}
