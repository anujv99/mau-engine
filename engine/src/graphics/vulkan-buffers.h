#pragma once

#include "common.h"

namespace mau {

  class Buffer: public HandledObject {
  public:
    Buffer(TUint64 buffer_size, VkBufferUsageFlags usage, VmaAllocationCreateFlags memory_flags = 0u);
    virtual ~Buffer();
  public:
    void* Map();
    void UnMap();

    inline VkBuffer Get() const { return m_Buffer; }
    inline const VkBuffer* Ref() const { return &m_Buffer; }
    inline TUint64 GetSize() const { return m_Size; }
  private:
    VkBuffer      m_Buffer       = VK_NULL_HANDLE;
    VmaAllocation m_Allocation   = VK_NULL_HANDLE;
    void*         m_MappedMemory = nullptr;
    TUint64       m_Size         = 0u;
  };

  class VertexBuffer: public Buffer {
  public:
    VertexBuffer(TUint64 buffer_size, void* data = nullptr);
    ~VertexBuffer();
  };

  class IndexBuffer: public Buffer {
  public:
    IndexBuffer(TUint64 buffer_size, void* data = nullptr);
    ~IndexBuffer();
  };

}
