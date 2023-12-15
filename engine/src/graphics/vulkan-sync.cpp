#include "vulkan-sync.h"

#include "vulkan-state.h"

namespace mau {

  Semaphore::Semaphore() {
    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0u;

    VK_CALL(vkCreateSemaphore(VulkanState::Ref().GetDevice(), &create_info,
                              nullptr, &m_Semaphore));
  }

  Semaphore::~Semaphore() {
    vkDestroySemaphore(VulkanState::Ref().GetDevice(), m_Semaphore, nullptr);
  }

  Fence::Fence(VkFenceCreateFlags flags) {
    VkFenceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;

    VK_CALL(vkCreateFence(VulkanState::Ref().GetDevice(), &create_info, nullptr,
                          &m_Fence));
  }

  Fence::~Fence() {
    vkDestroyFence(VulkanState::Ref().GetDevice(), m_Fence, nullptr);
  }

  void Fence::Reset() {
    VK_CALL(vkResetFences(VulkanState::Ref().GetDevice(), 1, &m_Fence));
  }

  void Fence::Wait() {
    MAU_PROFILE_SCOPR_COLOR("Fence::Wait", tracy::Color::Cyan);
    VK_CALL(vkWaitForFences(VulkanState::Ref().GetDevice(), 1, &m_Fence,
                            VK_TRUE, UINT64_MAX));
  }

} // namespace mau
