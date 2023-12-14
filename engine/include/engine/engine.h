#pragma once

#include <engine/utils/singleton.h>
#include <engine/window.h>
#include <engine/engine-config.h>
#include <engine/enums.h>

namespace mau {

  std::string GetAssetFolderPath();

  class Engine: public Singleton<Engine> {
    friend Singleton<Engine>;
  private:
    Engine(const EngineConfig& config);
    ~Engine();
  public:
    void Run() noexcept;
    void SetVulkanValidationLogSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept;
  private:
    Window m_Window;
    EngineConfig m_Config;
  };

}
