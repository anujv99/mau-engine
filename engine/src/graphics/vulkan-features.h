#pragma once

namespace mau {

  class VulkanFeatures {
  public:
    // make it configurable
    static inline bool IsRtEnabled() { return true; }
    static inline bool IsBufferDeviceAddressEnabled() { return true; }
  };

}
