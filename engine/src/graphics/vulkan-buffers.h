#pragma once

#include "common.h"

namespace mau {

  class CommandBuffer;

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
  protected:
    VkBuffer      m_Buffer       = VK_NULL_HANDLE;
    VmaAllocation m_Allocation   = VK_NULL_HANDLE;
    void*         m_MappedMemory = nullptr;
    TUint64       m_Size         = 0u;
  };

  class VertexBuffer: public Buffer {
  public:
    VertexBuffer(TUint64 buffer_size, const void* data = nullptr);
    ~VertexBuffer();
  };

  class IndexBuffer: public Buffer {
  public:
    IndexBuffer(TUint64 buffer_size, const void* data = nullptr);
    ~IndexBuffer();
  };

  class UniformBuffer: public Buffer {
  public:
    UniformBuffer(TUint64 buffer_size, const void* data = nullptr);
    virtual ~UniformBuffer();
  public:
    void Update(const void* data, TUint64 size);
    void Flush(Handle<CommandBuffer> cmd);
    VkDescriptorBufferInfo GetDescriptorInfo() const;
  private:
    bool m_IsUpdated = false;
  };

  // templated uniform buffer

  template <typename T>
  class StructuredUniformBuffer: public UniformBuffer {
  public:
    StructuredUniformBuffer(const T&& data);
    StructuredUniformBuffer();
    ~StructuredUniformBuffer();
  public:
    void Update(const T&& data);
  };

  template<typename T>
  inline StructuredUniformBuffer<T>::StructuredUniformBuffer(const T&& data): UniformBuffer(sizeof(T), &data) { }

  template<typename T>
  inline StructuredUniformBuffer<T>::StructuredUniformBuffer(): UniformBuffer(sizeof(T), nullptr) { }

  template<typename T>
  inline StructuredUniformBuffer<T>::~StructuredUniformBuffer() { }

  template<typename T>
  inline void StructuredUniformBuffer<T>::Update(const T&& data) {
    UniformBuffer::Update(&data, sizeof(T));
  }

}
