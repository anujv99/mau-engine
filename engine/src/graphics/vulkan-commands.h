#pragma once

#include <engine/types.h>
#include <vector>
#include "common.h"

namespace mau {

  class CommandBuffers {
    friend class CommandPool;
    CommandBuffers(VkCommandPool command_pool, VkDevice device, VkCommandBufferLevel level, TUint32 count);
  public:
    ~CommandBuffers();
  private:
    std::vector<VkCommandBuffer> m_CommandBuffers;
  };

  class CommandPool {
  public:
    CommandPool(VkDevice device, TUint32 queue_family_index, TUint32 flags);
    ~CommandPool();
  public:
    CommandBuffers AllocateCommandBuffers(TUint32 count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  private:
    VkDevice m_Device           = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
  };

}
