#pragma once

#include <string_view>
#include <engine/types.h>
#include <engine/enums.h>

namespace mau {

  struct EngineConfig {
    TUint32 Width                      = 0u;
    TUint32 Height                     = 0u;
    TUint32 ValidationSeverity         = 0u;
    TUint32 FramesInFlight             = 1u;
    std::string_view WindowName;
    std::string_view ApplicationName;
  };

}
