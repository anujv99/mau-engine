#include "pass.h"

#include <engine/log.h>

namespace mau {

  Pass::Pass(const String &name): m_Name(name) { }

  Pass::~Pass() { }

  bool Pass::Build(const UnorderedMap<String, Sink> &sinks,
                   TUint32                           swapchain_image_count) {
    for (const auto &source : m_Sources) {
      if (!sinks.contains(source.first)) {
        LOG_ERROR("failed to find source with name [%s]", source.first.c_str());
        return false;
      }

      auto it = m_Sources.find(source.first);
      it->second = sinks.at(source.first);
    }

    for (auto &sink : m_Sinks) {
      // check if sink just connect source resources
      if (sink.second.IsConnecting()) {
        const String &source_name = sink.second.GetInputSourceName();
        if (!m_Sources.contains(source_name)) {
          LOG_ERROR("cannot find source with name [%s] for sink [%s]",
                    source_name.c_str(), sink.first.c_str());
          return false;
        }

        // connect source resources with sink
        const Source &source = m_Sources.at(source_name);
        sink.second.AssignResources(source.GetResources());
      }
    }

    return PostBuild(swapchain_image_count);
  }

  void Pass::RegisterSource(const String &name) {
    if (m_Sources.contains(name)) {
      LOG_WARN("source with name [%s] already exists in pass [%s]",
               name.c_str(), m_Name.c_str());
      return;
    }

    m_Sources.insert(std::make_pair(name, Source(name)));
  }

  void Pass::RegisterSink(const String &input_source, const String &name) {
    if (m_Sinks.contains(name)) {
      LOG_WARN("sink with name [%s] already exists in pass [%s]", name.c_str(),
               m_Name.c_str());
      return;
    }

    m_Sinks.insert(std::make_pair(name, Sink(input_source, name)));
  }

} // namespace mau
