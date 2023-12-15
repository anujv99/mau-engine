#pragma once

#include <cstdlib>
#include <engine/log.h>

#define ASSERT(condition)                                                                                                                                                                              \
  {                                                                                                                                                                                                    \
    if (!(condition)) {                                                                                                                                                                                \
      LOG_FATAL("[" __FILE__ " : %d]"                                                                                                                                                                  \
                "assertion failed: " #condition,                                                                                                                                                       \
                __LINE__);                                                                                                                                                                             \
      std::abort();                                                                                                                                                                                    \
    }                                                                                                                                                                                                  \
  }
