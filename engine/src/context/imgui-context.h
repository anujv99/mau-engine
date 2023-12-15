#pragma once

#include <engine/events/event.h>
#include <engine/utils/singleton.h>
#include <imgui.h>

#include "../graphics/vulkan-state.h"
#include "../graphics/vulkan-renderpass.h"
#include "../graphics/vulkan-buffers.h"

namespace mau {

  class ImGuiContext: public Singleton<ImGuiContext> {
    friend class Singleton<ImGuiContext>;

  private:
    ImGuiContext(void *window, Handle<Renderpass> renderpass);
    ~ImGuiContext();

  public:
    void StartFrame();
    void OnEvent(Event &event);

    inline void BlockEvents(bool block) { m_BlockEvents = block; }

  private:
    void ImGuiDockspace();

  private:
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    bool             m_BlockEvents = false;
  };

} // namespace mau
