#include "vulkan-bindless.h"

#include "vulkan-state.h"
#include "vulkan-buffers.h"

namespace mau {

  VulkanBindless::VulkanBindless() {
    const VkDescriptorType descriptor_types[] = {
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    // create descriptor pool
    VkDescriptorPoolSize pool_sizes[ARRAY_SIZE(descriptor_types)] = {};

    for (size_t i = 0; i < ARRAY_SIZE(descriptor_types); i++) {
      pool_sizes[i] = {
        .type            = descriptor_types[i],
        .descriptorCount = m_DescriptorCount,
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
      VkDescriptorSetLayoutBinding binding = {
        .binding            = 0u,
        .descriptorType     = descriptor_types[i],
        .descriptorCount    = m_DescriptorCount,
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

      m_DescriptorIndexMap.insert(std::make_pair(descriptor_types[i], static_cast<TUint32>(i)));
      m_DescriptorLayouts.push_back(descriptor_layout);
      m_DescriptorSets.push_back(descriptor_set);
    }
  }

  VulkanBindless::~VulkanBindless() {
    for (const auto& layout : m_DescriptorLayouts) {
      vkDestroyDescriptorSetLayout(VulkanState::Ref().GetDevice(), layout, nullptr);
    }
    vkDestroyDescriptorPool(VulkanState::Ref().GetDevice(), m_DescriptorPool, nullptr);
  }

  TextureHandle VulkanBindless::AddTexture(const Handle<Texture>& texture) {
    const TUint32 sampler_set_index = m_DescriptorIndexMap[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
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
    const TUint32 buffer_set_index = m_DescriptorIndexMap[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER];
    const VkDescriptorSet buffer_set = m_DescriptorSets[buffer_set_index];
    ASSERT(buffer_set);

    BufferHandle handle = m_CurrentBufferIndex++;

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

}
