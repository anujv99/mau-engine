#pragma once

#include <cstdlib>
#include <engine/log.h>

#define ASSERT(condition)                                                      \
  {                                                                            \
    if (!(condition)) {                                                        \
      LOG_FATAL("assertion failed: " #condition);                              \
      std::abort();                                                            \
    }                                                                          \
  }
