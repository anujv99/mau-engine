#pragma once

#include <engine/types.h>

namespace mau {

  class RawImage {
  public:
    RawImage(const String& image_path);
    ~RawImage();
  public:
    TUint32 Width  = 0u;
    TUint32 Height = 0u;
    void*   Data   = nullptr;
  };

}
