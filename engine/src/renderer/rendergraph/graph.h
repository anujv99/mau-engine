#pragma once

#include "graphics/vulkan-commands.h"
#include "pass.h"

namespace mau {

  class RenderGraph: public HandledObject {
  public:
    RenderGraph();
    ~RenderGraph();
  public:
    void AddPass(Handle<Pass> pass);
    void Build();
    void Execute(Handle<CommandBuffer> cmd, TUint32 current_Frame);
  private:
    std::vector<Handle<Pass>>  m_Passes      = {};
    UnorderedMap<String, Sink> m_GlobalSinks = {};
  };

}
