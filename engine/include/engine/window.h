#pragma once

#include <string_view>
#include <engine/types.h>

namespace mau {

  class Window {
    friend class Engine;
  private:
    Window(TUint32 width, TUint32 height, std::string_view name);
    ~Window();
  public:
    inline const void* getRawWindow() const { return m_InternalState; }
  private:
    bool ShouldClose() const noexcept;
    void PollEvents() const noexcept;
  private:
    void* m_InternalState = nullptr;

    TUint32 m_Width  = 0u;
    TUint32 m_Height = 0u;
  };

}
