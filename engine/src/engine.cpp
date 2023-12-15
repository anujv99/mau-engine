#include <engine/engine.h>

#include <chrono>
#include <glm/glm.hpp>
#include <imgui.h>
#include <engine/log.h>
#include <engine/profiler.h>
#include <engine/input/input.h>

#include "context/imgui-context.h"
#include "renderer/renderer.h"
#include "graphics/vulkan-bindless.h"
#include "scene/internal-components.h"

namespace mau {

  EngineConfig validate_config(const EngineConfig &config) {
    EngineConfig output_config = config;

    if (output_config.FramesInFlight == 0u) {
      LOG_ERROR("cannot have 0 frames in flight, using 1");
      output_config.FramesInFlight = 1u;
    }

    return output_config;
  }

  Engine::Engine(const EngineConfig &config): m_Config(validate_config(config)), m_Window(config.Width, config.Height, config.WindowName) {

    m_Window.RegisterEventCallback(BIND_EVENT_FN(Engine::OnEvent));

#ifdef MAU_DEBUG
    bool enable_validation = true;
#else
    bool enable_validation = false;
#endif

    VulkanState::Create(enable_validation);
    VulkanState::Ref().SetValidationSeverity(config.ValidationSeverity);
    VulkanState::Ref().Init(config.ApplicationName, m_Window.GetRawWindow());

    VulkanBindless::Create();

    Renderer::Create(m_Window.GetRawWindow());

    String model_path = GetAssetFolderPath() + "assets/models/Sponza/glTF/Sponza.gltf";

    Handle<Mesh> mesh = make_handle<Mesh>(model_path);

    m_Scene = make_handle<Scene>();
    Entity bag = m_Scene->CreateEntity("Bag");

    TransformComponent &transform = bag.Get<TransformComponent>();
    transform.Position = glm::vec3(0.0f, 0.0f, 2.0f);
    transform.Rotation = glm::vec3(0.0f, 2.853, 0.0f);

    bag.Add<MeshComponent>(mesh);
  };

  Engine::~Engine() {
    m_Scene = nullptr;
    Renderer::Destroy();
    VulkanBindless::Destroy();
    VulkanState::Destroy();
  };

  void Engine::Run() noexcept {
    uint32_t frame_counter = 0;
    uint32_t last_second_framerate = 0;
    auto     last_time = std::chrono::high_resolution_clock::now();
    float    passed_time = 0.0f;
    float    delta_time = 0.0f;

    while (!m_Window.ShouldClose()) {
      MAU_FRAME_MARK();
      MAU_PROFILE_SCOPE("Engine::Loop");

      auto current_time = std::chrono::high_resolution_clock::now();
      auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_time);
      delta_time = (float)dt.count() * (float)1e-9;

      OnUpdate(delta_time);

      Renderer::Ref().StartFrame();

      Renderer::Ref().SubmitScene(m_Scene);

      ImGuiSceneList();

      Renderer::Ref().EndFrame();

      Input::OnUpdate();
      m_Window.PollEvents();

      // framerate
      last_time = current_time;
      passed_time += delta_time;
      frame_counter++;

      if (passed_time > 1.0f) {
        passed_time = 1.0f - passed_time;
        last_second_framerate = frame_counter;
        frame_counter = 0;
      }
    }

    vkDeviceWaitIdle(VulkanState::Ref().GetDevice());
  }

  void Engine::SetVulkanValidationLogSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept { VulkanState::Ref().SetValidationSeverity(severity, enabled); }

  void Engine::PushLayer(Handle<Layer> layer) noexcept { m_LayerStack.PushLayer(layer); }
  void Engine::PushOverlay(Handle<Layer> overlay) noexcept { m_OverlayStack.PushLayer(overlay); }
  void Engine::PopLayer(const String &name) noexcept { m_LayerStack.PopLayer(name); }
  void Engine::PopOverlay(const String &name) noexcept { m_OverlayStack.PopLayer(name); }

  void Engine::ImGuiSceneList() {
    static entt::entity selected_entity;

    if (ImGui::Begin("Scene List")) {

      m_Scene->Each([](Entity entity) -> void {
        NameComponent &name = entity.Get<NameComponent>();
        const bool     is_selected = selected_entity == entity.GetId();

        if (ImGui::Selectable(name.Name.c_str(), is_selected)) {
          selected_entity = entity.GetId();
        }

        if (is_selected) {
          TransformComponent &transform = entity.Get<TransformComponent>();

          if (ImGui::DragFloat3("Position", &transform.Position[0], 0.01f))
            transform.Updated = true;
          if (ImGui::DragFloat3("Rotation", &transform.Rotation[0], 0.01f))
            transform.Updated = true;
        }
      });
    }
    ImGui::End();

    // ImGui::ShowDemoWindow();
  }

  void Engine::OnEvent(Event &e) {
    ImGuiContext::Ref().OnEvent(e);
    Input::OnEvent(e);

    m_OverlayStack.OnEvent(e);
    m_LayerStack.OnEvent(e);
  }

  void Engine::OnUpdate(TFloat32 dt) {
    // set updated to false
    m_Scene->Each([](Entity entity) -> void {
      TransformComponent &transform = entity.Get<TransformComponent>();
      if (transform.Updated)
        transform.Updated = false;
    });

    // update layers
    m_OverlayStack.OnUpdate(dt);
    m_LayerStack.OnUpdate(dt);
  }

  std::string GetAssetFolderPath() { return MAU_ASSET_FOLDER; }

} // namespace mau
