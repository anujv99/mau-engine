#include "vulkan-commands.h"

#include <engine/assert.h>
#include "vulkan-state.h"

namespace mau {

  CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer, VkCommandPool m_CommandPool): m_CommandBuffer(command_buffer), m_CommandPool(m_CommandPool) {
    ASSERT(m_CommandBuffer != VK_NULL_HANDLE);
    ASSERT(m_CommandPool != VK_NULL_HANDLE);
  }

  CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(VulkanState::Ref().GetDevice(), m_CommandPool, 1, &m_CommandBuffer);
  }

  void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext                    = nullptr;
    begin_info.flags                    = flags;
    begin_info.pInheritanceInfo         = nullptr;

    VK_CALL(vkBeginCommandBuffer(m_CommandBuffer, &begin_info));
  }

  void CommandBuffer::End() {
    VK_CALL(vkEndCommandBuffer(m_CommandBuffer));
  }

  void CommandBuffer::Reset() {
    vkResetCommandBuffer(m_CommandBuffer, 0u);
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

  std::vector<Handle<CommandBuffer>> CommandPool::AllocateCommandBuffers(TUint32 count, VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext                       = nullptr;
    alloc_info.commandPool                 = m_CommandPool;
    alloc_info.level                       = level;
    alloc_info.commandBufferCount          = count;

    std::vector<VkCommandBuffer> vk_command_buffers(static_cast<size_t>(count));
    std::vector<Handle<CommandBuffer>> command_buffers;
    command_buffers.reserve(vk_command_buffers.size());
    VK_CALL(vkAllocateCommandBuffers(m_Device, &alloc_info, vk_command_buffers.data()));

    for (const VkCommandBuffer& buffer : vk_command_buffers) {
      command_buffers.emplace_back(new CommandBuffer(buffer, m_CommandPool));
    }

    return command_buffers;
  }

}
