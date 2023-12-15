#pragma once

#include <engine/types.h>
#include "common.h"

namespace mau {

  class Semaphore: public HandledObject {
  public:
    Semaphore();
    ~Semaphore();

  public:
    inline VkSemaphore        Get() const { return m_Semaphore; }
    inline const VkSemaphore *Ref() const { return &m_Semaphore; }

  private:
    VkSemaphore m_Semaphore = VK_NULL_HANDLE;
  };

  class Fence: public HandledObject {
  public:
    Fence(VkFenceCreateFlags flags = 0u);
    ~Fence();

  public:
    void Reset();
    void Wait();

  public:
    inline VkFence        Get() const { return m_Fence; }
    inline const VkFence *Ref() const { return &m_Fence; }

  private:
    VkFence m_Fence = VK_NULL_HANDLE;
  };

} // namespace mau
