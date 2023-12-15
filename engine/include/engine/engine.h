#pragma once

#include <engine/events/event.h>
#include <engine/utils/singleton.h>
#include <engine/window.h>
#include <engine/engine-config.h>
#include <engine/enums.h>
#include <engine/scene/scene.h>
#include <engine/core/layers.h>

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

    void PushLayer(Handle<Layer> layer) noexcept;
    void PushOverlay(Handle<Layer> overlay) noexcept;
    void PopLayer(const String &name) noexcept;
    void PopOverlay(const String &name) noexcept;

  private:
    void ImGuiSceneList();
    void OnEvent(Event &event);
    void OnUpdate(TFloat32 dt);

  private:
    Window       m_Window;
    EngineConfig m_Config;

    Handle<Scene> m_Scene;

    // layers & overlays
    LayerStack m_LayerStack;
    LayerStack m_OverlayStack;
  };

} // namespace mau
