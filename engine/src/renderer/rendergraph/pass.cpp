#include "pass.h"

#include <engine/log.h>

namespace mau {

  Pass::Pass(const String& name): m_Name(name) { }

  Pass::~Pass() { }

  bool Pass::Build(const UnorderedMap<String, Sink>& sinks, TUint32 swapchain_image_count) {
    for (const auto& source : m_Sources) {
      if (!sinks.contains(source.first)) {
        LOG_ERROR("failed to find source with name [%s]", source.first.c_str());
        return false;
      }

      auto it = m_Sources.find(source.first);
      it->second = sinks.at(source.first);
    }

    return PostBuild(swapchain_image_count);
  }

  void Pass::RegisterSource(const String& name) {
    if (m_Sources.contains(name)) {
      LOG_WARN("source with name [%s] already exists in pass [%s]", name.c_str(), m_Name.c_str());
      return;
    }

    m_Sources.insert(std::make_pair(name, Source(name)));
  }

  void Pass::RegisterSink(const String& name) {
    if (m_Sinks.contains(name)) {
      LOG_WARN("sink with name [%s] already exists in pass [%s]", name.c_str(), m_Name.c_str());
      return;
    }

    m_Sinks.insert(std::make_pair(name, Sink(name)));
  }
  
}
