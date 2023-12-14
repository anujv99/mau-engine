#include "vulkan-commands.h"

#include <engine/assert.h>

namespace mau {

  CommandBuffers::CommandBuffers(VkCommandPool command_pool, VkDevice device, VkCommandBufferLevel level, TUint32 count) {
    ASSERT(command_pool != VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext                       = nullptr;
    alloc_info.commandPool                 = command_pool;
    alloc_info.level                       = level;
    alloc_info.commandBufferCount          = count;

    m_CommandBuffers.resize(count);
    VK_CALL(vkAllocateCommandBuffers(device, &alloc_info, m_CommandBuffers.data()));
  }

  CommandBuffers::~CommandBuffers() {
    m_CommandBuffers.clear();
  }

  CommandPool::CommandPool(VkDevice device, TUint32 queue_family_index, TUint32 flags):
    m_Device(device) {
    ASSERT(m_Device != VK_NULL_HANDLE);

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.pNext                   = nullptr;
    create_info.flags                   = flags;
    create_info.queueFamilyIndex        = queue_family_index;

    VK_CALL(vkCreateCommandPool(m_Device, &create_info, nullptr, &m_CommandPool));
  }

  CommandPool::~CommandPool() {
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
  }

  CommandBuffers CommandPool::AllocateCommandBuffers(TUint32 count, VkCommandBufferLevel level) {
    return CommandBuffers(m_CommandPool, m_Device, level, count);
  }

}
