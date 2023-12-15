#pragma once

#include <engine/types.h>
#include <engine/events/event.h>

namespace mau {

  class WindowCloseEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::WINDOW_CLOSE);
    REGISTER_EVENT_LOG("WindowCloseEvent", "closed");

  public:
    WindowCloseEvent(): Event(EventType::WINDOW_CLOSE){};
    ~WindowCloseEvent() = default;
  };

  class WindowResizeEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::WINDOW_RESIZE);
    REGISTER_EVENT_LOG("WindowResizeEvent", "width: %d, height: %d", m_Width,
                       m_Height);

  public:
    WindowResizeEvent(TUint32 width, TUint32 height)
        : Event(EventType::WINDOW_RESIZE), m_Width(width), m_Height(height){};
    ~WindowResizeEvent() = default;

    TUint32 GetWidth() const { return m_Width; }
    TUint32 GetHeight() const { return m_Height; }

  private:
    TUint32 m_Width = 0u;
    TUint32 m_Height = 0u;
  };

} // namespace mau
