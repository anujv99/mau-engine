#pragma once

#include <engine/types.h>
#include "source.h"
#include "sink.h"

namespace mau {

  class Pass {
  protected:
    Pass(const String& name);
    virtual ~Pass();
  protected:
    void RegisterSource(const String& name, const Source& source);
    void RegisterSink(const String& name, const Sink& sink);
  protected:
    const String                 m_Name    = "";
    UnorderedMap<String, Source> m_Sources = {};
    UnorderedMap<String, Sink>   m_Sinks   = {};
  };

}
