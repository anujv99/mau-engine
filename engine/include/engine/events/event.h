#pragma once

#include <functional>
#include <engine/log.h>

#define REGISTER_EVENT_TYPE(type)                                              \
public:                                                                        \
  static EventType GetStaticType() { return type; }
#define REGISTER_EVENT_LOG(name, fmt, ...)                                     \
public:                                                                        \
  void Log() const { LOG_INFO("%s: [" fmt "]", name, ##__VA_ARGS__); }

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace mau {

  enum class EventType {
    NONE = 0,
    WINDOW_CLOSE,
    WINDOW_RESIZE,
    KEY_PRESS,
    KEY_RELEASE,
    KEY_REPEAT,
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_MOVE,
    MOUSE_SCROLL,
  };

  class Event {
    friend class EventDispatcher;

  protected:
    Event(EventType type): m_Type(type){};

  public:
    virtual ~Event() = default;

    EventType   GetType() const { return m_Type; }
    inline void Handle() { m_Handled = true; }

    virtual void Log() const = 0;

  private:
    EventType m_Type = EventType::NONE;
    bool      m_Handled = false;
  };

  class EventDispatcher {
  public:
    EventDispatcher(Event &e): m_Event(e){};
    ~EventDispatcher() = default;

  public:
    template <class T> void Dispatch(std::function<void(T &)> func) {
      if (!m_Event.m_Handled && m_Event.GetType() == T::GetStaticType()) {
        T &event = static_cast<T &>(m_Event);
        func(event);
      }
    }

  private:
    Event &m_Event;
  };

} // namespace mau
