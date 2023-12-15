#pragma once

#include <functional>
#include <string_view>
#include <engine/types.h>
#include <engine/events/event.h>

namespace mau {

  using EventEmitFunction = std::function<void(Event &)>;

  class Window {
    friend class Engine;

  private:
    Window(TUint32 width, TUint32 height, std::string_view name);
    ~Window();

  public:
    inline void             *GetRawWindow() const { return m_InternalState; }
    inline EventEmitFunction GetEventCallback() const { return m_EventCallback; }

  private:
    bool ShouldClose() const noexcept;
    void PollEvents() const noexcept;

    void RegisterEventCallback(EventEmitFunction callback) noexcept;

  private:
    void *m_InternalState = nullptr;

    // this function will be called whenever the
    // window want to dispatch an event
    EventEmitFunction m_EventCallback;

    TUint32 m_Width = 0u;
    TUint32 m_Height = 0u;
  };

} // namespace mau
