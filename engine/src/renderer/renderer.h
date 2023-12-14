#pragma once

#include <engine/types.h>
#include <engine/utils/singleton.h>
#include <engine/engine.h>
#include <glm/glm.hpp>

#include "../graphics/vulkan-state.h"
#include "../graphics/vulkan-renderpass.h"
#include "../graphics/vulkan-shaders.h"
#include "../graphics/vulkan-pipeline.h"
#include "../graphics/vulkan-image.h"
#include "../graphics/vulkan-buffers.h"
#include "../graphics/vulkan-push-constant.h"
#include "../scene/mesh.h"
#include "../scene/camera.h"

#include "renderer/rendergraph/graph.h"
#include "renderer/rendergraph/sink.h"

namespace mau {

  struct VertexShaderData {
    glm::vec4 color;
    glm::mat4 mvp;
    TUint32   ubo_index;
  };

  struct ShaderData {
    glm::mat4 model;
  };

  class Renderer: public Singleton<Renderer> {
    friend class Singleton<Renderer>;
    Renderer(void* window_ptr);
    ~Renderer();
  public:
    void StartFrame();
    void EndFrame();
    void Render(Handle<CommandBuffer> cmd, TUint32 frame_index);
  private:
    void RecordCommandBuffer(TUint64 idx);
    void ImGuiTest(TUint32 idx);
    void CreateViewportBuffers(TUint32 width, TUint32 height);
    void CreateImguiTextures();
  private:
    TUint64    m_CurrentFrame = 0u;
    VkExtent2D m_Extent       = {};

    Handle<VertexShader>               m_VertexShader   = nullptr;
    Handle<FragmentShader>             m_FragmentShader = nullptr;
    Handle<Pipeline>                   m_Pipeline       = nullptr;
    std::vector<Handle<CommandBuffer>> m_CommandBuffers = {};
    std::vector<Handle<Semaphore>>     m_ImageAvailable = {};
    std::vector<Handle<Semaphore>>     m_RenderFinished = {};
    std::vector<Handle<Fence>>         m_QueueSubmit    = {};

    Handle<RenderGraph>                m_Rendergraph    = nullptr;

    // temp
    Handle<Mesh> m_Mesh = nullptr;
    Handle<Texture> m_Texture = nullptr;
    TUint32 m_TextureHandle = 0u;
    Handle<PushConstant<VertexShaderData>> m_PushConstant = nullptr;
    Handle<StructuredUniformBuffer<ShaderData>> m_UniformBuffer = nullptr;

    Sink sink_color = Sink("imgui-viewport-color");
    Sink sink_depth = Sink("imgui-viewport-depth");
    Sampler sampler;
    std::vector<void*> imgui_texture_ids = {};

    Camera m_Camera;

    TUint32 m_ImGuiViewportWidth  = 800u;
    TUint32 m_ImGuiViewportHeight = 600u;

    TUint32 m_CurrentViewportWidth  = 800u;
    TUint32 m_CurrentViewportHeight = 600u;
  };
  
}
