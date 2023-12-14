#pragma once

#include <engine/types.h>
#include <engine/events/event.h>

namespace mau {

  class MousePressEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::MOUSE_PRESS);
    REGISTER_EVENT_LOG("MousePressEvent", "button: %d", m_Button);
  public:
    MousePressEvent(TUint32 button): Event(EventType::MOUSE_PRESS), m_Button(button) { };
    ~MousePressEvent() = default;

    TUint32 GetButton() const { return m_Button; }
  private:
    TUint32 m_Button = 0u;
  };

  class MouseReleaseEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::MOUSE_RELEASE);
    REGISTER_EVENT_LOG("MouseReleaseEvent", "button: %d", m_Button);
  public:
    MouseReleaseEvent(TUint32 button): Event(EventType::MOUSE_RELEASE), m_Button(button) { };
    ~MouseReleaseEvent() = default;

    TUint32 GetButton() const { return m_Button; }
  private:
    TUint32 m_Button = 0u;
  };

  class MouseMoveEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::MOUSE_MOVE);
    REGISTER_EVENT_LOG("MouseMoveEvent", "x: %f, y: %f", m_X, m_Y);
  public:
    MouseMoveEvent(TFloat32 x, TFloat32 y): Event(EventType::MOUSE_MOVE), m_X(x), m_Y(y) { };
    ~MouseMoveEvent() = default;

    TFloat32 GetX() const { return m_X; }
    TFloat32 GetY() const { return m_Y; }
  private:
    TFloat32 m_X = 0.0f;
    TFloat32 m_Y = 0.0f;
  };

  class MouseScrollEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::MOUSE_SCROLL);
    REGISTER_EVENT_LOG("MouseScrollEvent", "x: %f, y: %f", m_X, m_Y);
  public:
    MouseScrollEvent(TFloat32 x, TFloat32 y): Event(EventType::MOUSE_SCROLL), m_X(x), m_Y(y) { };
    ~MouseScrollEvent() = default;

    TFloat32 GetX() const { return m_X; }
    TFloat32 GetY() const { return m_Y; }
  private:
    TFloat32 m_X = 0.0f;
    TFloat32 m_Y = 0.0f;
  };

}

