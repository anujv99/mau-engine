#pragma once

#include <engine/types.h>
#include "../graphics/vulkan-image.h"

namespace mau {

  class Material: HandledObject {
  public:
		Material(const String&& texture_path);
		~Material() = default;
  private:
		Handle<Image>     m_Image     = nullptr;
		Handle<ImageView> m_ImageView = nullptr;
		Handle<Sampler>   m_Sampler   = nullptr;
  };

}
