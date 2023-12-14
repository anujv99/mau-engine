#pragma once

#include <engine/types.h>
#include <engine/utils/singleton.h>

#include "common.h"

namespace mau {

  class Texture;
  class UniformBuffer;
  class AccelerationBuffer;
  class ImageView;

  template <typename T>
  class StructuredUniformBuffer;

  enum class BindlessDescriptorType {
    UNIFORM                = 0,
    TEXTURE                = 1,
    MATERIAL               = 2,
    STORAGE_IMAGE          = 3,
    ACCELERATION_STRUCTURE = 4,
  };

  class VulkanBindless: public Singleton<VulkanBindless> {
    friend class Singleton<VulkanBindless>;
  private:
    VulkanBindless();
    ~VulkanBindless();
  public:
    TextureHandle AddTexture(const Handle<Texture>& texture);
    BufferHandle AddBuffer(const Handle<UniformBuffer>& buffer);
    MaterialHandle AddMaterial(const GPUMaterial& material);
    ImageHandle AddStorageImage(const Handle<ImageView>& image_view);
    AccelerationStructureHandle AddAccelerationStructure(const Handle<AccelerationBuffer>& accel_struct);
  public:
    inline const std::vector<VkDescriptorSetLayout>& GetDescriptorLayout() const { return m_DescriptorLayouts; }
    inline const std::vector<VkDescriptorSet>& GetDescriptorSet() const { return m_DescriptorSets; }
  private:
    BufferHandle AddBufferInternal(const Handle<UniformBuffer>& buffer, BindlessDescriptorType type, TUint32 array_index);
    void SetupMaterialBuffer();
  private:
    const TUint32 m_DescriptorCount         = 1024u;
    TUint32       m_CurrentTextureIndex     = 0u;
    TUint32       m_CurrentBufferIndex      = 0u;
    TUint32       m_CurrentMaterialIndex    = 0u;
    TUint32       m_CurrentImageIndex       = 0u;
    TUint32       m_CurrentAccelStructIndex = 0u;

    VkDescriptorPool                                    m_DescriptorPool     = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout>                  m_DescriptorLayouts  = {};
    std::vector<VkDescriptorSet>                        m_DescriptorSets     = {};
    std::unordered_map<BindlessDescriptorType, TUint32> m_DescriptorIndexMap = {};

    Handle<StructuredUniformBuffer<GPUMaterial>>        m_MaterialBuffer     = nullptr;
  };
  
}
