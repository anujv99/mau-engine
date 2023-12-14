#pragma once

#include <utility>
#include <functional>

#include <engine/assert.h>
#include <engine/types.h>
#include <engine/memory.h>

namespace mau {

  class HandledObject {
    template <typename>
    friend class Handle;
  protected:
    HandledObject() {};
    virtual ~HandledObject() {};
  private:
    inline void AddRef() { m_RefCount++; }
    inline void RemRef() { m_RefCount--; }

    inline TUint32 RefCount() const { return m_RefCount; }
  private:
    TUint32 m_RefCount = 0u;
  };

  template <class T>
  class Handle final {
  public:
    Handle() {}
    Handle(T* instance);
    Handle(const Handle& other);
    ~Handle() { Destroy(); }
  public:
    inline operator bool() const { return m_Instance != nullptr; }
    inline bool operator==(const Handle& other) const { return m_Instance == other.m_Instance; }
    inline bool operator!=(const Handle& other) const { return m_Instance != other.m_Instance; }
    inline bool operator==(const T* const other) const { return m_Instance == other; }
    inline bool operator!=(const T* const other) const { return m_Instance != other; }
    inline void operator=(const Handle& other);
    inline void operator=(T* const other);
    inline T* operator->() const;
    inline T& operator*() const;

    template <class C>
    inline operator Handle<C>();
  private:
    void Destroy();
  private:
    T* m_Instance = nullptr;
  };

  template<class T>
  inline Handle<T>::Handle(T* instance) {
    if (instance) {
      m_Instance = instance;
      m_Instance->AddRef();
    }
  }

  template<class T>
  inline Handle<T>::Handle(const Handle& other) {
    if (other) {
      m_Instance = other.m_Instance;
      m_Instance->AddRef();
    }
  }

  template<class T>
  inline void Handle<T>::operator=(const Handle& other) {
    if (other) {
      Destroy();
      m_Instance = other.m_Instance;
      m_Instance->AddRef();
    }
  }

  template<class T>
  inline void Handle<T>::operator=(T* const other) {
    Destroy();

    m_Instance = other;
    if (m_Instance)
      m_Instance->AddRef();
  }

  template<class T>
  inline T* Handle<T>::operator->() const {
    ASSERT(m_Instance);
    return m_Instance;
  }

  template<class T>
  inline T& Handle<T>::operator*() const {
    ASSERT(m_Instance);
    return *m_Instance;
  }

  template<class T>
  inline void Handle<T>::Destroy() {
    if (m_Instance) {
      m_Instance->RemRef();

      if (m_Instance->RefCount() == 0) {
        HandledObject* base_class = dynamic_cast<HandledObject*>(m_Instance);
        MAU_FREE(base_class);
      }

      m_Instance = nullptr;
    }
  }

  template <class T, typename... Args>
  Handle<T> make_handle(Args... args) {
    T* ptr = nullptr;
    MAU_ALLOC(ptr, T, std::forward<Args>(args)...);
    return ptr;
  }

  template<class T>
  template<class C>
  inline Handle<T>::operator Handle<C>() {
    if (m_Instance == nullptr) return Handle<C>(nullptr);

    C* ptr = dynamic_cast<C*>(m_Instance);
    ASSERT(ptr != nullptr);
    return Handle<C>(ptr);
  }

}
