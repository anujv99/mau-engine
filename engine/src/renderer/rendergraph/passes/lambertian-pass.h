#pragma once

#include "renderer/rendergraph/pass.h"
#include "graphics/vulkan-image.h"
#include "graphics/vulkan-renderpass.h"

namespace mau {

  class LambertianPass: public Pass {
  public:
    LambertianPass();
    ~LambertianPass();
  public:
    inline Handle<Renderpass> GetRenderpass() const { return m_Renderpass; } // TODO: remove
  protected:
    bool PostBuild(TUint32 swapchain_image_count) override;
    void Execute(Handle<CommandBuffer> cmd, TUint32 frame_index) override;
  private:
    Handle<Renderpass>               m_Renderpass   = nullptr;
    std::vector<Handle<Framebuffer>> m_Framebuffers = {};
    TUint32                          m_Width        = 0u;
    TUint32                          m_Height       = 0u;
  };

}
