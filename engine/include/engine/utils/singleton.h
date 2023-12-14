#pragma once

namespace mau {

  template <class T>
  class Singleton {
  public:
    template <typename... Args>
    static void Create(Args... args);
    static void Destroy();

    inline static T* Get() { return m_Instance; }
    inline static T& Ref() { return *m_Instance; }
  private:
    static T* m_Instance;
  };

  template <class T>
  template <typename... Args>
  void Singleton<T>::Create(Args... args) {
    Destroy();
    m_Instance = new T(args...);
  }

  template <class T>
  void Singleton<T>::Destroy() {
    if (m_Instance != nullptr) delete m_Instance;
    m_Instance = nullptr;
  }

  template <class T>
  T* Singleton<T>::m_Instance = nullptr;

}
