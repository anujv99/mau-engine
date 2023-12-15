#include "image-loader.h"

#include <engine/log.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace mau {

  RawImage::RawImage(const String &image_path) {
    int      width = 0, height = 0, channels = 0;
    stbi_uc *pixels = stbi_load(image_path.c_str(), &width, &height, &channels,
                                STBI_rgb_alpha);

    if (!pixels) {
      LOG_ERROR("failed to load image: %s", image_path.c_str());
      return;
    }

    Data = reinterpret_cast<void *>(pixels);
    Width = static_cast<TUint32>(width);
    Height = static_cast<TUint32>(height);
  }

  RawImage::~RawImage() {
    if (Data) {
      stbi_image_free(Data);
      Data = nullptr;
    }
  }

} // namespace mau
