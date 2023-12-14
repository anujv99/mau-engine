#pragma once

#include "common.h"
#include "vulkan-sync.h"
#include "vulkan-commands.h"
#include "vulkan-swapchain.h"

namespace mau {

  class VulkanQueue: public HandledObject {
    template <class T, typename... Args>
    friend Handle<T> make_handle(Args... args);
  protected:
    VulkanQueue(VkQueue queue, VkDevice device);
  public:
    virtual ~VulkanQueue();
  public:
    void Submit(Handle<CommandBuffer> cmd, VkPipelineStageFlags wait_stages, Handle<Semaphore> wait_semaphore, Handle<Semaphore> signal_semaphore, Handle<Fence> signal_fence);
    void Submit(Handle<CommandBuffer> cmd);
    void WaitIdle();

    VkQueue Get() const { return m_Queue; }
    const VkQueue* const Ref() const { return &m_Queue; }
  protected:
    VkQueue        m_Queue     = VK_NULL_HANDLE;
    VkDevice       m_Device    = VK_NULL_HANDLE;
  };

  class PresentQueue final: public VulkanQueue {
    template <class T, typename... Args>
    friend Handle<T> make_handle(Args... args);
    PresentQueue(VkQueue queue, VkDevice device);
  public:
    ~PresentQueue();
  public:
    void Present(TUint32 image_index, Handle<VulkanSwapchain> swapchain, Handle<Semaphore> wait_semaphore);
  };

}
