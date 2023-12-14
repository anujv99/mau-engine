#pragma once

#include <engine/assert.h>
#include "resources.h"
#include "sink.h"

namespace mau {

  class Source {
  public:
    Source(const String& name): m_Name(name) { }
    Source(const Sink& sink): m_Name(sink.m_Name), m_Resources(sink.m_Resources) { }
    ~Source() { }
  public:
    inline const String& GetName() const { return m_Name; }
    inline const std::vector<Handle<Resource>>& GetResources() const { return m_Resources; }
    inline Handle<Resource> GetResource(TUint32 current_frame) const { ASSERT(current_frame < m_Resources.size()); return m_Resources[current_frame]; }
  private:
    String                        m_Name      = "";
    std::vector<Handle<Resource>> m_Resources = {}; // per frame
  };

}
