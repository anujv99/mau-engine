#pragma once

#include <engine/types.h>
#include <engine/utils/handle.h>
#include <engine/events/event.h>

namespace mau {

  class Layer: public HandledObject {
  public:
    Layer(const String &name): m_LayerName(name) { }
    virtual ~Layer() = default;

  public:
    virtual void OnAttach() = 0;
    virtual void OnDetach() = 0;
    virtual void OnUpdate(TFloat32 dt) = 0;
    virtual void OnEvent(Event &e) = 0;
    virtual void OnImgGiUpdate(){};

    inline const String &GetName() const { return m_LayerName; }

  private:
    String m_LayerName;
  };

  class LayerStack {
  public:
    LayerStack() = default;
    ~LayerStack();

  public:
    void PushLayer(Handle<Layer> layer);
    void PopLayer(const String &name);
    void OnUpdate(TFloat32 dt);
    void OnEvent(Event &e);
    void OnImGuiUpdate();

    Vector<Handle<Layer>>::iterator begin() { return m_Layers.begin(); }
    Vector<Handle<Layer>>::iterator end() { return m_Layers.end(); }

  private:
    Vector<Handle<Layer>> m_Layers;
  };

} // namespace mau
