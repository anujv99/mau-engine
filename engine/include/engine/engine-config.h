#pragma once

#include <string_view>
#include <engine/types.h>

namespace mau {

  struct EngineConfig {
    TUint32 Width;
    TUint32 Height;
    std::string_view WindowName;
    std::string_view ApplicationName;
  };

}
