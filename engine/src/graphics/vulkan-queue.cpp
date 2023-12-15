#include "vulkan-queue.h"

#include <engine/assert.h>

namespace mau {

  VulkanQueue::VulkanQueue(VkQueue queue, VkDevice device): m_Queue(queue), m_Device(device) {
    ASSERT(queue != VK_NULL_HANDLE);
    ASSERT(device != VK_NULL_HANDLE);
  }

  VulkanQueue::~VulkanQueue() { }

  void VulkanQueue::Submit(Handle<CommandBuffer> cmd, VkPipelineStageFlags wait_stages, Handle<Semaphore> wait_semaphore, Handle<Semaphore> signal_semaphore, Handle<Fence> signal_fence) {
    MAU_PROFILE_SCOPR_COLOR("VulkanQueue::Submit", tracy::Color::Cyan);
    ASSERT(cmd != nullptr);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
    submit_info.pWaitSemaphores = wait_semaphore ? wait_semaphore->Ref() : nullptr;
    submit_info.pWaitDstStageMask = &wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = cmd->Ref();
    submit_info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
    submit_info.pSignalSemaphores = signal_semaphore ? signal_semaphore->Ref() : nullptr;
    VK_CALL(vkQueueSubmit(m_Queue, 1, &submit_info, signal_fence ? signal_fence->Get() : VK_NULL_HANDLE));
  }

  void VulkanQueue::Submit(Handle<CommandBuffer> cmd) { Submit(cmd, 0u, nullptr, nullptr, nullptr); }

  void VulkanQueue::WaitIdle() { VK_CALL(vkQueueWaitIdle(m_Queue)); }

  PresentQueue::PresentQueue(VkQueue queue, VkDevice device): VulkanQueue(queue, device) { }

  PresentQueue::~PresentQueue() { }

  void PresentQueue::Present(TUint32 image_index, Handle<VulkanSwapchain> swapchain, Handle<Semaphore> wait_semaphore) {
    MAU_PROFILE_SCOPR_COLOR("PresentQueue::Present", tracy::Color::Cyan);
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = wait_semaphore->Ref();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchain->Ref();
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(m_Queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      return;
    } else {
      VK_CALL(result);
    }
  }

} // namespace mau
