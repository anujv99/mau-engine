#include <engine/engine.h>

using namespace mau;

int main() {
  EngineConfig config;
  config.Width = 1280u;
  config.Height = 720u;
  config.WindowName = "Mau Engine";

  Engine::Create(config);

  Engine::Ref().Run();

  Engine::Destroy();
  return 0;
}
