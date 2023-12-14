#include "vulkan-buffers.h"

#include "vulkan-state.h"

namespace mau {

  void UploadUsingStaging(Buffer* dst, void* data) {
    ASSERT(dst && data);
    Buffer staging_buffer(dst->GetSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* buffer_mem = staging_buffer.Map();
    memcpy(buffer_mem, data, dst->GetSize());
    staging_buffer.UnMap();

    Handle<CommandBuffer> cmd = VulkanState::Ref().GetCommandPool(VK_QUEUE_TRANSFER_BIT)->AllocateCommandBuffers(1)[0];
    cmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferCopy copy_region = {};
    copy_region.srcOffset    = 0ui64;
    copy_region.dstOffset    = 0ui64;
    copy_region.size         = dst->GetSize();
    vkCmdCopyBuffer(cmd->Get(), staging_buffer.Get(), dst->Get(), 1u, &copy_region);

    cmd->End();

    VkSubmitInfo submit_info         = {};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                = nullptr;
    submit_info.waitSemaphoreCount   = 0u;
    submit_info.pWaitSemaphores      = nullptr;
    submit_info.pWaitDstStageMask    = nullptr;
    submit_info.commandBufferCount   = 1u;
    submit_info.pCommandBuffers      = cmd->Ref();
    submit_info.signalSemaphoreCount = 0u;
    submit_info.pSignalSemaphores    = nullptr;

    VkQueue transfer_queue = VulkanState::Ref().GetDeviceHandle()->GetTransferQueue();

    VK_CALL(vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE));
    vkQueueWaitIdle(transfer_queue);
  }

  Buffer::Buffer(TUint64 buffer_size, VkBufferUsageFlags usage, VmaAllocationCreateFlags memory_flags): m_Size(buffer_size) {
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage                   = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags                   = memory_flags;

    VkBufferCreateInfo create_info    = {};
    create_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext                 = nullptr;
    create_info.flags                 = 0u;
    create_info.size                  = buffer_size;
    create_info.usage                 = usage;
    create_info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0u;
    create_info.pQueueFamilyIndices   = nullptr;

    VK_CALL(vmaCreateBuffer(VulkanState::Ref().GetVulkanMemoryAllocator(), &create_info, &alloc_info, &m_Buffer, &m_Allocation, nullptr));
  }

  Buffer::~Buffer() {
    vmaDestroyBuffer(VulkanState::Ref().GetVulkanMemoryAllocator(), m_Buffer, m_Allocation);
  }

  void* Buffer::Map() {
    if (m_MappedMemory == nullptr) {
      VK_CALL(vmaMapMemory(VulkanState::Ref().GetVulkanMemoryAllocator(), m_Allocation, &m_MappedMemory));
    }
    return m_MappedMemory;
  }

  void Buffer::UnMap() {
    if (m_MappedMemory != nullptr) {
      vmaUnmapMemory(VulkanState::Ref().GetVulkanMemoryAllocator(), m_Allocation);
      m_MappedMemory = nullptr;
    }
  }

  VertexBuffer::VertexBuffer(TUint64 buffer_size, void* data):
    Buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) {
    
    if (data) {
      UploadUsingStaging(this, data);
    }
  }

  VertexBuffer::~VertexBuffer() { }

  IndexBuffer::IndexBuffer(TUint64 buffer_size, void* data):
    Buffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) {

    if (data) {
      UploadUsingStaging(this, data);
    }
  }

  IndexBuffer::~IndexBuffer() { }

}
