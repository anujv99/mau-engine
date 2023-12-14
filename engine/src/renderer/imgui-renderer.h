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
    ImguiRenderer(void* window, Handle<Renderpass> renderpass);
    ~ImguiRenderer();
  public:
    void StartFrame();
  private:
    void ImGuiDockspace();
  private:
    VkDescriptorPool  m_DescriptorPool = VK_NULL_HANDLE;
  };

}
