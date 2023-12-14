#pragma once

#include <engine/types.h>
#include <engine/utils/singleton.h>

#include "common.h"

namespace mau {

  class Texture;
  class UniformBuffer;

  class VulkanBindless: public Singleton<VulkanBindless> {
    friend class Singleton<VulkanBindless>;
  private:
    VulkanBindless();
    ~VulkanBindless();
  public:
    TextureHandle AddTexture(const Handle<Texture>& texture);
    BufferHandle AddBuffer(const Handle<UniformBuffer>& buffer);
  public:
    inline const std::vector<VkDescriptorSetLayout>& GetDescriptorLayout() const { return m_DescriptorLayouts; }
    inline const std::vector<VkDescriptorSet>& GetDescriptorSet() const { return m_DescriptorSets; }
  private:
    const TUint32 m_DescriptorCount     = 1024u;
    TUint32       m_CurrentTextureIndex = 0u;
    TUint32       m_CurrentBufferIndex  = 0u;

    VkDescriptorPool                              m_DescriptorPool     = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout>            m_DescriptorLayouts  = {};
    std::vector<VkDescriptorSet>                  m_DescriptorSets     = {};
    std::unordered_map<VkDescriptorType, TUint32> m_DescriptorIndexMap = {};
  };
  
}
