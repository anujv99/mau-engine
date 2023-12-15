#pragma once

#include <engine/types.h>
#include "graphics/vulkan-buffers.h"
#include "graphics/vulkan-image.h"

namespace mau {

  struct MaterialCreateInfo {
    String DiffuseMap;
    String NormalMap;
  };

  class Material: public HandledObject {
  public:
    Material(const MaterialCreateInfo &create_info);
    ~Material() = default;

  public:
    inline MaterialHandle GetMaterialHandle() const { return m_MaterialHandle; }

  private:
    Handle<Texture> m_Diffuse = nullptr;
    Handle<Texture> m_Normal = nullptr;
    MaterialHandle  m_MaterialHandle = UINT32_MAX;
  };

} // namespace mau
