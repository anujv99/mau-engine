#include "material.h"

#include "graphics/vulkan-bindless.h"

namespace mau {

  Material::Material(const MaterialCreateInfo& create_info) {
    GPUMaterial material = {};

    if (create_info.DiffuseMap != "") {
      m_Diffuse = make_handle<Texture>(create_info.DiffuseMap);
      material.Diffuse = VulkanBindless::Ref().AddTexture(m_Diffuse);
    }

    if (create_info.NormalMap != "") {
      m_Normal = make_handle<Texture>(create_info.NormalMap);
      material.Normal = VulkanBindless::Ref().AddTexture(m_Normal);
    }

    m_MaterialHandle = VulkanBindless::Ref().AddMaterial(material);
  }

}
