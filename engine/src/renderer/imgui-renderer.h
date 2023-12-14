#pragma once

#include <engine/utils/singleton.h>
#include <imgui.h>
#include "../graphics/vulkan-state.h"
#include "../graphics/vulkan-renderpass.h"
#include "../graphics/vulkan-buffers.h"

namespace mau {

  class ImguiRenderer: public Singleton<ImguiRenderer> {
    friend class Singleton<ImguiRenderer>;
  private:
    ImguiRenderer(void* window);
    ~ImguiRenderer();
  public:
    void StartFrame();
    void EndFrame(Handle<CommandBuffer> cmd, TUint64 idx);
  private:
    VkDescriptorPool                 m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<Handle<Framebuffer>> m_Framebuffers   = {};
    VkExtent2D                       m_Extent         = {};
    Handle<Renderpass>               m_Renderpass     = nullptr;
  };

}
