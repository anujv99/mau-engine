#include <engine/engine.h>

#include <chrono>
#include <glm/glm.hpp>
#include <imgui.h>
#include <engine/log.h>
#include <engine/profiler.h>

#include "renderer/renderer.h"
#include "graphics/vulkan-bindless.h"
#include "scene/internal-components.h"
#include "graphics/vulkan-features.h"

namespace mau {

  // temp
  void imgui_fps_overlay(uint32_t framerate) {
    static int location = 1;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
    window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
    window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
    window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    window_flags |= ImGuiWindowFlags_NoMove;

    bool open = true;
    if (ImGui::Begin("Example: Simple overlay", &open, window_flags))
    {
      ImGui::Text("Metrics");
      ImGui::Separator();
      float dt = io.DeltaTime * 1000.0f;
      ImGui::Text("Delta Time: %.5fms", io.DeltaTime * 1000.0f);
      ImGui::Text("FPS:        %d", (int)framerate);
    }
    ImGui::End();
  }

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

    #ifdef MAU_DEBUG
      bool enable_validation = true;
    #else
      bool enable_validation = false;
    #endif

    VulkanState::Create(enable_validation);
    VulkanState::Ref().SetValidationSeverity(config.ValidationSeverity);
    VulkanState::Ref().Init(config.ApplicationName, m_Window.getRawWindow());

    VulkanBindless::Create();

    Renderer::Create(m_Window.getRawWindow());

    String model_path = GetAssetFolderPath() + "assets/models/ABeautifulGame/glTF/ABeautifulGame.gltf";

    Handle<Mesh> mesh = make_handle<Mesh>(model_path);

    m_Scene = make_handle<Scene>();
    Entity bag = m_Scene->CreateEntity("Bag");

    // TransformComponent& transform = bag.Get<TransformComponent>();
    // transform.Position = glm::vec3(0.0f, -1.31f, 4.46f);
    // transform.Rotation = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);

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
    auto last_time = std::chrono::high_resolution_clock::now();
    float passed_time = 0.0f;
    float delta_time = 0.0f;

    while (!m_Window.ShouldClose()) {
      MAU_FRAME_MARK();
      MAU_PROFILE_SCOPE("Engine::Loop");

      Renderer::Ref().StartFrame();

      Renderer::Ref().SubmitScene(m_Scene);

      imgui_fps_overlay(last_second_framerate);
      ImGuiSceneList();

      Renderer::Ref().EndFrame();
      
      m_Window.PollEvents();

      // framerate
      auto current_time = std::chrono::high_resolution_clock::now();
      auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - last_time);
      delta_time = (float)dt.count() * (float)1e-9;
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

  void Engine::SetVulkanValidationLogSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept {
    VulkanState::Ref().SetValidationSeverity(severity, enabled);
  }

  void Engine::ImGuiSceneList() {
    static entt::entity selected_entity;

    if (ImGui::Begin("Scene List")) {
      
      m_Scene->Each([](Entity entity) -> void {
        NameComponent& name = entity.Get<NameComponent>();
        const bool is_selected = selected_entity == entity.GetId();

        if (ImGui::Selectable(name.Name.c_str(), is_selected)) {
          selected_entity = entity.GetId();
        }

        if (is_selected) {
          TransformComponent& transform = entity.Get<TransformComponent>();

          ImGui::DragFloat3("Position", &transform.Position[0], 0.01f);
          ImGui::DragFloat3("Rotation", &transform.Rotation[0], 0.01f);
        }
      });

    }
    ImGui::End();

    // ImGui::ShowDemoWindow();
  }

  std::string GetAssetFolderPath() {
    return MAU_ASSET_FOLDER;
  }

}
