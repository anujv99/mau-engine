#pragma once

#include <engine/types.h>
#include <engine/events/event.h>

namespace mau {

  class KeyPressEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::KEY_PRESS);
    REGISTER_EVENT_LOG("KeyPressEvent", "key: %d", m_Key);

  public:
    KeyPressEvent(TUint32 key): Event(EventType::KEY_PRESS), m_Key(key){};
    ~KeyPressEvent() = default;

    TUint32 GetKey() const { return m_Key; }

  private:
    TUint32 m_Key = 0u;
  };

  class KeyReleaseEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::KEY_RELEASE);
    REGISTER_EVENT_LOG("KeyReleaseEvent", "key: %d", m_Key);

  public:
    KeyReleaseEvent(TUint32 key): Event(EventType::KEY_RELEASE), m_Key(key){};
    ~KeyReleaseEvent() = default;

    TUint32 GetKey() const { return m_Key; }

  private:
    TUint32 m_Key = 0u;
  };

  class KeyRepeatEvent: public Event {
    REGISTER_EVENT_TYPE(EventType::KEY_REPEAT);
    REGISTER_EVENT_LOG("KeyRepeatEvent", "key: %d", m_Key);

  public:
    KeyRepeatEvent(TUint32 key): Event(EventType::KEY_REPEAT), m_Key(key){};
    ~KeyRepeatEvent() = default;

    TUint32 GetKey() const { return m_Key; }

  private:
    TUint32 m_Key = 0u;
  };

} // namespace mau
