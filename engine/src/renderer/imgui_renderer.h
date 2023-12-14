#pragma once

#include <engine/utils/singleton.h>
#include <imgui.h>
#include "../graphics/vulkan-state.h"

namespace mau {

  class ImguiRenderer: public Singleton<ImguiRenderer> {
    friend class Singleton<ImguiRenderer>;
  private:
    ImguiRenderer(Handle<Renderpass> renderpass, void* window);
    ~ImguiRenderer();
  public:
    void StartFrame();
    void EndFrame(Handle<CommandBuffer> buffer);
  private:
    VkDescriptorPool   m_DescriptorPool = VK_NULL_HANDLE;
  };

}
