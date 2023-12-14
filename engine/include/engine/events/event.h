#pragma once

#define REGISTER_EVENT_TYPE(type) public: static EventType GetStaticType() { return type; }

namespace mau {

  enum class EventType {
    NONE = 0,
    WINDOW_CLOSE,
    WINDOW_RESIZE,
  };

  class Event {
  protected:
    Event(EventType type): m_Type(type) { };
  public:
    virtual ~Event() = default;

    EventType GetType() const { return m_Type; }
  private:
    EventType m_Type = EventType::NONE;
  };

  class EventDispatcher {
  public:
    EventDispatcher(const Event& e): m_Event(e) { };
    ~EventDispatcher() = default;
  public:
    template<class T>
    bool Dispatch() {
      if (m_Event.GetType() == T::GetStaticType())
        return true;
      return false;
    }
  private:
    const Event& m_Event;
  };

}

