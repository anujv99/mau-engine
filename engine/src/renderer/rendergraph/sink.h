#pragma once

#include "resources.h"

namespace mau {

  class Sink {
    friend class Source;
  public:
    Sink(const String& name): m_Name(name) { }
    ~Sink() { }
  public:
    void AssignResources(const std::vector<Handle<Resource>>& resources) { m_Resources = resources; }

    inline const String& GetName() const { return m_Name; }
  private:
    const String                  m_Name      = "";
    std::vector<Handle<Resource>> m_Resources = {}; // per frame
  };

}
