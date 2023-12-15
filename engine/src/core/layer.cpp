
#include <engine/log.h>
#include <engine/core/layers.h>

namespace mau {

  LayerStack::~LayerStack() { m_Layers.clear(); }

  void LayerStack::PushLayer(Handle<Layer> layer) {
    for (const auto &l : m_Layers) {
      if (l->GetName() == layer->GetName()) {
        LOG_ERROR("Layer with name %s already exists", layer->GetName().c_str());
        return;
      }
    }

    m_Layers.push_back(layer);
    layer->OnAttach();
  }

  void LayerStack::PopLayer(const String &name) {
    for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it) {
      if ((*it)->GetName() == name) {
        m_Layers.erase(it);
        (*it)->OnDetach();
        return;
      }
    }

    LOG_ERROR("Layer with name %s does not exist", name.c_str());
  }

  void LayerStack::OnUpdate(TFloat32 dt) {
    for (auto &layer : m_Layers)
      layer->OnUpdate(dt);
  }

  void LayerStack::OnEvent(Event &e) {
    for (auto &layer : m_Layers)
      layer->OnEvent(e);
  }

} // namespace mau
