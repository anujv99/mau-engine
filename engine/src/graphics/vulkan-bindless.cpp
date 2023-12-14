#include "vulkan-bindless.h"

#include "vulkan-state.h"
#include "vulkan-buffers.h"

namespace mau {

  VkDescriptorType get_descriptor_type(BindlessDescriptorType type) {
    switch (type)
    {
    case mau::BindlessDescriptorType::UNIFORM:                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case mau::BindlessDescriptorType::TEXTURE:                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case mau::BindlessDescriptorType::MATERIAL:               return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case mau::BindlessDescriptorType::STORAGE_IMAGE:          return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case mau::BindlessDescriptorType::ACCELERATION_STRUCTURE: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    case mau::BindlessDescriptorType::RT_OBJECT_DESC:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    default:
      break;
    }

    ASSERT(false);
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }

  VulkanBindless::VulkanBindless() {
    // [type, count]
    const std::tuple<BindlessDescriptorType, TUint32> descriptor_types[] = {
      { BindlessDescriptorType::UNIFORM, m_DescriptorCount },
      { BindlessDescriptorType::TEXTURE, m_DescriptorCount },
      { BindlessDescriptorType::MATERIAL, 1 },
      { BindlessDescriptorType::STORAGE_IMAGE, m_DescriptorCount },
      { BindlessDescriptorType::ACCELERATION_STRUCTURE, m_DescriptorCount },
      { BindlessDescriptorType::RT_OBJECT_DESC, 1 },
    };

    // create descriptor pool
    VkDescriptorPoolSize pool_sizes[ARRAY_SIZE(descriptor_types)] = {};

    for (size_t i = 0; i < ARRAY_SIZE(descriptor_types); i++) {
      pool_sizes[i] = {
        .type            = get_descriptor_type(std::get<0>(descriptor_types[i])),
        .descriptorCount = std::get<1>(descriptor_types[i]),
      };
    }

    VkDescriptorPoolCreateInfo pool_create_info = {
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0u,
      .maxSets       = static_cast<uint32_t>(ARRAY_SIZE(descriptor_types)),
      .poolSizeCount = static_cast<uint32_t>(ARRAY_SIZE(pool_sizes)),
      .pPoolSizes    = pool_sizes,
    };

    VK_CALL(vkCreateDescriptorPool(VulkanState::Ref().GetDevice(), &pool_create_info, nullptr, &m_DescriptorPool));

    // create descriptor sets, since only last binding can have dynamic size
    // we create separate sets for each descriptor type
    for (size_t i = 0; i < ARRAY_SIZE(descriptor_types); i++) {
      const BindlessDescriptorType type = std::get<0>(descriptor_types[i]);

      VkDescriptorSetLayoutBinding binding = {
        .binding            = 0u,
        .descriptorType     = get_descriptor_type(type),
        .descriptorCount    = std::get<1>(descriptor_types[i]),
        .stageFlags         = VK_SHADER_STAGE_ALL,
        .pImmutableSamplers = nullptr,
      };

      VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

      VkDescriptorSetLayoutBindingFlagsCreateInfo flag_info = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext         = nullptr,
        .bindingCount  = 1u,
        .pBindingFlags = &flags,
      };

      VkDescriptorSetLayoutCreateInfo set_layout_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = reinterpret_cast<const void*>(&flag_info),
        .flags        = 0u,
        .bindingCount = 1u,
        .pBindings    = &binding,
      };

      VkDescriptorSetLayout descriptor_layout = VK_NULL_HANDLE;
      VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
      VK_CALL(vkCreateDescriptorSetLayout(VulkanState::Ref().GetDevice(), &set_layout_info, nullptr, &descriptor_layout));

      VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = m_DescriptorPool,
        .descriptorSetCount = 1u,
        .pSetLayouts        = &descriptor_layout,
      };

      VK_CALL(vkAllocateDescriptorSets(VulkanState::Ref().GetDevice(), &alloc_info, &descriptor_set));

      m_DescriptorIndexMap.insert(std::make_pair(type, static_cast<TUint32>(i)));
      m_DescriptorLayouts.push_back(descriptor_layout);
      m_DescriptorSets.push_back(descriptor_set);
    }

    SetupMaterialBuffer();
    SetupRTObjectBuffer();
  }

  VulkanBindless::~VulkanBindless() {
    for (const auto& layout : m_DescriptorLayouts) {
      vkDestroyDescriptorSetLayout(VulkanState::Ref().GetDevice(), layout, nullptr);
    }
    vkDestroyDescriptorPool(VulkanState::Ref().GetDevice(), m_DescriptorPool, nullptr);
  }

  TextureHandle VulkanBindless::AddTexture(const Handle<Texture>& texture) {
    const TUint32 sampler_set_index = m_DescriptorIndexMap[BindlessDescriptorType::TEXTURE];
    const VkDescriptorSet sampler_set = m_DescriptorSets[sampler_set_index];
    ASSERT(sampler_set);

    TextureHandle handle = m_CurrentTextureIndex++;

    VkDescriptorImageInfo image_descriptor_info = texture->GetDescriptorInfo();
    VkWriteDescriptorSet write_descriptor_set   = {};
    write_descriptor_set.sType                  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.pNext                  = nullptr;
    write_descriptor_set.dstSet                 = sampler_set;
    write_descriptor_set.dstBinding             = 0u;
    write_descriptor_set.dstArrayElement        = handle;
    write_descriptor_set.descriptorCount        = 1u;
    write_descriptor_set.descriptorType         = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.pImageInfo             = &image_descriptor_info;

    vkUpdateDescriptorSets(VulkanState::Ref().GetDevice(), 1u, &write_descriptor_set, 0u, nullptr);

    return handle;
  }

  BufferHandle VulkanBindless::AddBuffer(const Handle<UniformBuffer>& buffer) {
    return AddBufferInternal(buffer, BindlessDescriptorType::UNIFORM, m_CurrentBufferIndex++);
  }

  MaterialHandle VulkanBindless::AddMaterial(const GPUMaterial& material) {
    MaterialHandle handle = m_CurrentMaterialIndex++;
    m_MaterialBuffer->UpdateIndex(material, handle);
    return handle;
  }

  ImageHandle VulkanBindless::AddStorageImage(const Handle<ImageView>& image_view) {
    const TUint32 storage_image_set_index = m_DescriptorIndexMap[BindlessDescriptorType::STORAGE_IMAGE];
    const VkDescriptorSet storage_image_set = m_DescriptorSets[storage_image_set_index];
    ASSERT(storage_image_set);

    ImageHandle handle = m_CurrentImageIndex++;

    VkDescriptorImageInfo image_descriptor_info = {
      .sampler     = VK_NULL_HANDLE,
      .imageView   = image_view->GetImageView(),
      .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };

    VkWriteDescriptorSet write_descriptor_set   = {};
    write_descriptor_set.sType                  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.pNext                  = nullptr;
    write_descriptor_set.dstSet                 = storage_image_set;
    write_descriptor_set.dstBinding             = 0u;
    write_descriptor_set.dstArrayElement        = handle;
    write_descriptor_set.descriptorCount        = 1u;
    write_descriptor_set.descriptorType         = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write_descriptor_set.pImageInfo             = &image_descriptor_info;

    vkUpdateDescriptorSets(VulkanState::Ref().GetDevice(), 1u, &write_descriptor_set, 0u, nullptr);

    return handle;
  }

  AccelerationStructureHandle VulkanBindless::AddAccelerationStructure(const Handle<AccelerationBuffer>& accel_struct) {
    const TUint32 accel_struct_set_index = m_DescriptorIndexMap[BindlessDescriptorType::ACCELERATION_STRUCTURE];
    const VkDescriptorSet accel_struct_set = m_DescriptorSets[accel_struct_set_index];
    ASSERT(accel_struct_set);

    AccelerationStructureHandle handle = m_CurrentAccelStructIndex++;

    VkAccelerationStructureKHR tlas = accel_struct->GetTLAS();
    VkWriteDescriptorSetAccelerationStructureKHR accel_struct_descriptor_info = {
      .sType                      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
      .pNext                      = nullptr,
      .accelerationStructureCount = 1u,
      .pAccelerationStructures    = &tlas,
    };

    VkWriteDescriptorSet write_descriptor_set   = {};
    write_descriptor_set.sType                  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.pNext                  = &accel_struct_descriptor_info;
    write_descriptor_set.dstSet                 = accel_struct_set;
    write_descriptor_set.dstBinding             = 0u;
    write_descriptor_set.dstArrayElement        = handle;
    write_descriptor_set.descriptorCount        = 1u;
    write_descriptor_set.descriptorType         = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    vkUpdateDescriptorSets(VulkanState::Ref().GetDevice(), 1u, &write_descriptor_set, 0u, nullptr);

    return handle;
  }

  RTObjectHandle VulkanBindless::AddRTObject(const RTObjectDesc& desc) {
    RTObjectHandle handle = m_CurrentRTObjectDescIndex++;
    m_RTObjectDesc->UpdateIndex(desc, handle);
    return handle;
  }

  BufferHandle VulkanBindless::AddBufferInternal(const Handle<UniformBuffer>& buffer, BindlessDescriptorType type, TUint32 array_index) {
    const TUint32 buffer_set_index = m_DescriptorIndexMap[type];
    const VkDescriptorSet buffer_set = m_DescriptorSets[buffer_set_index];
    ASSERT(buffer_set);

    BufferHandle handle = array_index;

    VkDescriptorBufferInfo buffer_descriptor_info = buffer->GetDescriptorInfo();
    VkWriteDescriptorSet write_descriptor_set   = {};
    write_descriptor_set.sType                  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.pNext                  = nullptr;
    write_descriptor_set.dstSet                 = buffer_set;
    write_descriptor_set.dstBinding             = 0u;
    write_descriptor_set.dstArrayElement        = handle;
    write_descriptor_set.descriptorCount        = 1u;
    write_descriptor_set.descriptorType         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.pBufferInfo            = &buffer_descriptor_info;

    vkUpdateDescriptorSets(VulkanState::Ref().GetDevice(), 1u, &write_descriptor_set, 0u, nullptr);

    return handle;
  }

  void VulkanBindless::SetupMaterialBuffer() {
    m_MaterialBuffer = make_handle<StructuredUniformBuffer<GPUMaterial>>(m_DescriptorCount);
    AddBufferInternal(m_MaterialBuffer, BindlessDescriptorType::MATERIAL, 0);
  }

  void VulkanBindless::SetupRTObjectBuffer() {
    m_RTObjectDesc = make_handle<StructuredUniformBuffer<RTObjectDesc>>(m_DescriptorCount);
    AddBufferInternal(m_RTObjectDesc, BindlessDescriptorType::RT_OBJECT_DESC, 0);
  }

}
