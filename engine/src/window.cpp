#include <engine/window.h>
#include <engine/exceptions.h>
#include <engine/log.h>

#include <GLFW/glfw3.h>

#define GLFW_CALL(call, reason) {                  \
  int ret = call;                                  \
  if (ret != GLFW_TRUE) {                          \
    const char* error = nullptr;                   \
    int err_code = glfwGetError(&error);           \
    if (err_code != GLFW_NO_ERROR) {               \
      LOG_FATAL("%s [reason: %s]", reason, error); \
    } else {                                       \
      LOG_FATAL("%s [reason: unknown]", reason);   \
    }                                              \
    throw WindowException(reason);                 \
  }                                                \
}                                                  \

namespace mau {

  void glfw_error_callback(int error, const char* description) {
    LOG_ERROR("glfw error: %s", description);
  }

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

}
