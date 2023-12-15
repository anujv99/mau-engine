#pragma once

#define BIT(x) 1 << x

namespace mau {

  enum VulkanValidationLogSeverity {
    VERBOSE = BIT(0),
    INFO = BIT(1),
    WARNING = BIT(2),
    ERROR = BIT(3),
    ALL = BIT(0) | BIT(1) | BIT(2) | BIT(3),
  };

  enum ShaderType {
    VERTEX = BIT(0),
    FRAGMENT = BIT(1),
  };

} // namespace mau
