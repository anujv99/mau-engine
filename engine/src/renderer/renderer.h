#pragma once

#include <engine/types.h>
#include <engine/utils/singleton.h>
#include <engine/engine.h>

#include "../graphics/vulkan-state.h"
#include "../graphics/vulkan-renderpass.h"
#include "../graphics/vulkan-shaders.h"
#include "../graphics/vulkan-pipeline.h"
#include "../graphics/vulkan-image.h"
#include "../graphics/vulkan-buffers.h"

namespace mau {

  class Renderer: public Singleton<Renderer> {
    friend class Singleton<Renderer>;
    Renderer(void* window_ptr);
    ~Renderer();
  public:
    void StartFrame();
    void EndFrame();
  private:
    void RecordCommandBuffer(TUint64 idx);
  private:
    TUint64    m_CurrentFrame = 0u;
    VkExtent2D m_Extent       = {};

    Handle<Renderpass>                 m_Renderpass     = nullptr;
    Handle<VertexShader>               m_VertexShader   = nullptr;
    Handle<FragmentShader>             m_FragmentShader = nullptr;
    Handle<Pipeline>                   m_Pipeline       = nullptr;
    std::vector<Handle<Framebuffer>>   m_Framebuffers   = {};
    std::vector<Handle<CommandBuffer>> m_CommandBuffers = {};
    std::vector<Handle<Semaphore>>     m_ImageAvailable = {};
    std::vector<Handle<Semaphore>>     m_RenderFinished = {};
    std::vector<Handle<Fence>>         m_QueueSubmit    = {};

    // temp
    Handle<VertexBuffer>               m_QuadBuffer     = nullptr;
    Handle<IndexBuffer>                m_QuadIndices    = nullptr;
  };
  
}
