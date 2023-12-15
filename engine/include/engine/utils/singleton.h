#pragma once

#include <engine/memory.h>

namespace mau {

  template <class T> class Singleton {
  public:
    template <typename... Args> static void Create(Args... args);
    static void                             Destroy();

    inline static T *Get() { return m_Instance; }
    inline static T &Ref() { return *m_Instance; }

  private:
    static T *m_Instance;
  };

  template <class T> template <typename... Args> inline void Singleton<T>::Create(Args... args) {
    Destroy();
    MAU_ALLOC(m_Instance, T, args...);
  }

  template <class T> inline void Singleton<T>::Destroy() {
    if (m_Instance != nullptr) {
      MAU_FREE(m_Instance);
    }
  }

  template <class T> T *Singleton<T>::m_Instance = nullptr;

} // namespace mau
