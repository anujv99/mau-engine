#include <engine/engine.h>

#include <engine/log.h>
#include "graphics/vulkan-state.h"

namespace mau {

  EngineConfig validate_config(const EngineConfig& config) {
    EngineConfig output_config = config;

    if (output_config.FramesInFlight == 0u) {
      LOG_ERROR("cannot have 0 frames in flight, using 1");
      output_config.FramesInFlight = 1u;
    }

    return output_config;
  }

  Engine::Engine(const EngineConfig& config):
    m_Config(validate_config(config)),
    m_Window(config.Width, config.Height, config.WindowName) {

    VulkanState::Create(MAU_DEBUG);
    VulkanState::Ref().SetValidationSeverity(config.ValidationSeverity);
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

  void Engine::SetVulkanValidationLogSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept {
    VulkanState::Ref().SetValidationSeverity(severity, enabled);
  }

}
