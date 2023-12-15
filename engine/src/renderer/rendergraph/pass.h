#pragma once

#include <engine/types.h>
#include "source.h"
#include "sink.h"
#include "graphics/vulkan-commands.h"

namespace mau {

  class Pass: public HandledObject {
  protected:
    Pass(const String &name);

  public:
    virtual ~Pass();

  public:
    bool         Build(const UnorderedMap<String, Sink> &sinks, TUint32 swapchain_image_count);
    virtual void Execute(Handle<CommandBuffer> cmd, TUint32 frame_index) = 0;

    inline const UnorderedMap<String, Sink> &GetSinks() const { return m_Sinks; }

  protected:
    void         RegisterSource(const String &name);
    void         RegisterSink(const String &input_source, const String &name);
    virtual bool PostBuild(TUint32 swapchain_image_count) = 0;

  protected:
    const String                 m_Name = "";
    UnorderedMap<String, Source> m_Sources = {};
    UnorderedMap<String, Sink>   m_Sinks = {};
  };

} // namespace mau
