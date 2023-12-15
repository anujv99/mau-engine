#pragma once

#include "resources.h"

namespace mau {

  class Sink {
    friend class Source;

  public:
    Sink(const String &name): m_Name(name), m_InputSourceName(name), m_Connecting(false) { }
    Sink(const String &input_name, const String &name): m_Name(name), m_InputSourceName(input_name), m_Connecting(true) { }
    ~Sink() { }

  public:
    void AssignResources(const std::vector<Handle<Resource>> &resources) { m_Resources = resources; }

    inline const String    &GetName() const { return m_Name; }
    inline const String    &GetInputSourceName() const { return m_InputSourceName; }
    inline bool             IsConnecting() const { return m_Connecting; }
    inline Handle<Resource> GetResource(TUint32 current_frame) const {
      ASSERT(current_frame < m_Resources.size());
      return m_Resources[current_frame];
    }

  private:
    const String                  m_InputSourceName = "";
    const String                  m_Name = "";
    const bool                    m_Connecting = false;
    std::vector<Handle<Resource>> m_Resources = {}; // per frame
  };

} // namespace mau
