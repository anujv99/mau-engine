#include <engine/engine.h>
#include <engine/enums.h>
#include <engine/log.h>
#include <engine/exceptions.h>

using namespace mau;

int main() {
  EngineConfig config;
  config.Width  = 1920u;
  config.Height = 1080u;
  config.FramesInFlight = 3u;
  config.WindowName = "Mau Engine";
  config.ValidationSeverity = VulkanValidationLogSeverity::ERROR | VulkanValidationLogSeverity::WARNING;

  try {
    Engine::Create(config);

    Engine::Ref().Run();

    Engine::Destroy();
  } catch (GraphicsException e) {
    LOG_FATAL("%s", e.what().data());
  } catch (WindowException e) {
    LOG_FATAL("%s", e.what().data());
  } catch (std::exception e) {
    LOG_FATAL("%s", e.what());
  }

  return 0;
}
