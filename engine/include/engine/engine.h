#pragma once

#include <engine/utils/singleton.h>
#include <engine/window.h>
#include <engine/engine-config.h>

namespace mau {

  class Engine: public Singleton<Engine> {
    friend Singleton<Engine>;
  private:
    Engine(const EngineConfig& config);
    ~Engine();
  public:
    void Run() noexcept;
  private:
    Window m_Window;
    EngineConfig m_Config;
  };

}
