#pragma once

#include <engine/events/event.h>
#include <engine/utils/singleton.h>
#include <engine/window.h>
#include <engine/engine-config.h>
#include <engine/enums.h>
#include <engine/scene/scene.h>

namespace mau {

  std::string GetAssetFolderPath();

  class Engine: public Singleton<Engine> {
    friend Singleton<Engine>;

  private:
    Engine(const EngineConfig &config);
    ~Engine();

  public:
    void Run() noexcept;
    void SetVulkanValidationLogSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept;

  private:
    void ImGuiSceneList();
    void OnEvent(Event &event);

  private:
    Window       m_Window;
    EngineConfig m_Config;

    Handle<Scene> m_Scene;
  };

} // namespace mau
