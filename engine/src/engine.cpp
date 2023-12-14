#include <engine/engine.h>

#include "graphics/vulkan-state.h"

namespace mau {

  Engine::Engine(const EngineConfig& config):
    m_Config(config),
    m_Window(config.Width, config.Height, config.WindowName) {

    VulkanState::Create(MAU_DEBUG);
    VulkanState::Ref().Init(config.ApplicationName, m_Window.getRawWindow());
  };

  Engine::~Engine() {
    VulkanState::Destroy();
  };

  void Engine::Run() noexcept {
    while (!m_Window.ShouldClose()) {

      m_Window.PollEvents();
    }
  }

}
