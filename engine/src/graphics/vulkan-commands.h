#pragma once

#include <engine/types.h>
#include <vector>
#include "common.h"

namespace mau {

  class CommandBuffer: public HandledObject {
    friend class CommandPool;
    CommandBuffer(VkCommandBuffer command_buffer, VkCommandPool m_CommandPool);
  public:
    ~CommandBuffer();
  public:
    void Begin(VkCommandBufferUsageFlags flags = 0u);
    void End();
    void Reset();

    inline VkCommandBuffer Get() const { return m_CommandBuffer; }
    inline const VkCommandBuffer* Ref() const { return &m_CommandBuffer; }
  private:
    VkCommandBuffer m_CommandBuffer;
    VkCommandPool   m_CommandPool;
  };

  class CommandPool: public HandledObject {
  public:
    CommandPool(VkDevice device, TUint32 queue_family_index, TUint32 flags);
    ~CommandPool();
  public:
    std::vector<Handle<CommandBuffer>> AllocateCommandBuffers(TUint32 count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  private:
    VkDevice m_Device           = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
  };

}
