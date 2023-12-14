#pragma once

#include <glm/glm.hpp>
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
    VkDeviceAddress GetDeviceAddress();

    inline VkBuffer Get() const { return m_Buffer; }
    inline const VkBuffer* Ref() const { return &m_Buffer; }
    inline TUint64 GetSize() const { return m_Size; }
  protected:
    VkBuffer        m_Buffer        = VK_NULL_HANDLE;
    VmaAllocation   m_Allocation    = VK_NULL_HANDLE;
    void*           m_MappedMemory  = nullptr;
    TUint64         m_Size          = 0u;
    VkDeviceAddress m_DeviceAddress = 0u;
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
    void Update(const void* data, TUint64 size, TUint64 offset = 0u);
    void Flush(Handle<CommandBuffer> cmd);
    VkDescriptorBufferInfo GetDescriptorInfo() const;
  private:
    bool m_IsUpdated = false;
  };

  // acceleration structure for ray tracing

  struct AccelerationBufferCreateInfo {
    Handle<VertexBuffer> Vertices       = nullptr;
    Handle<IndexBuffer>  Indices        = nullptr;
    TUint32              VertexSize     = 0u;
    TUint32              PositionOffset = 0u;
    TUint32              VertexCount    = 0u;
    TUint32              IndexCount     = 0u;
    TUint32              CustomIndex    = 0u;
  };

  class BottomLevelAS: public HandledObject {
  public:
    BottomLevelAS(const AccelerationBufferCreateInfo& create_info);
    ~BottomLevelAS();
  public:
    inline VkAccelerationStructureKHR GetBLAS() const { return m_BLAS; }
    inline TUint32 GetCustomIndex() const { return m_CustomIndex; }
  private:
    void BuildBLAS(const AccelerationBufferCreateInfo& create_info);
  private:
    Handle<VertexBuffer>       m_VertexBuffer      = nullptr;
    Handle<IndexBuffer>        m_IndexBufffer      = nullptr;
    Handle<Buffer>             m_BLASBuffer        = nullptr;
    VkAccelerationStructureKHR m_BLAS              = VK_NULL_HANDLE;
    TUint32                    m_CustomIndex       = 0u;
  };

  class AccelerationBuffer: public HandledObject {
  public:
    AccelerationBuffer(const Vector<Handle<BottomLevelAS>>& blases);
    ~AccelerationBuffer();
  public:
    void UpdateTransform(const glm::mat4& transform, Handle<CommandBuffer> cmd);
  public:
    inline VkAccelerationStructureKHR GetTLAS() const { return m_TLAS; }
  private:
    void BuildTLAS(Handle<CommandBuffer> cmd = nullptr, bool update = false, glm::mat4 transform = glm::mat4(1.0f));
  private:
    Vector<Handle<BottomLevelAS>> m_BLASes            = {};
    Handle<Buffer>                m_TLASBuffer        = nullptr;
    Handle<Buffer>                m_InstanceBuffer    = nullptr;
    Handle<Buffer>                m_TLASScratchBuffer = nullptr;
    VkAccelerationStructureKHR    m_TLAS              = VK_NULL_HANDLE;
  };

  // templated uniform buffer

  template <typename T>
  class StructuredUniformBuffer: public UniformBuffer {
  public:
    StructuredUniformBuffer(const T&& data);
    StructuredUniformBuffer(TUint32 array_size);
    StructuredUniformBuffer();
    ~StructuredUniformBuffer();
  public:
    void Update(const T&& data);
    void UpdateIndex(const T& data, TUint32 index);
  private:
    const TUint32 m_ArraySize = 1u;
  };

  template<typename T>
  inline StructuredUniformBuffer<T>::StructuredUniformBuffer(const T&& data): m_ArraySize(1u), UniformBuffer(sizeof(T), &data) { }

  template<typename T>
  inline StructuredUniformBuffer<T>::StructuredUniformBuffer(TUint32 array_size): m_ArraySize(array_size), UniformBuffer(sizeof(T) * static_cast<size_t>(array_size)) { }

  template<typename T>
  inline StructuredUniformBuffer<T>::StructuredUniformBuffer(): UniformBuffer(sizeof(T), nullptr) { }

  template<typename T>
  inline StructuredUniformBuffer<T>::~StructuredUniformBuffer() { }

  template<typename T>
  inline void StructuredUniformBuffer<T>::Update(const T&& data) {
    UniformBuffer::Update(&data, sizeof(T));
  }

  template<typename T>
  inline void StructuredUniformBuffer<T>::UpdateIndex(const T& data, TUint32 index) {
    ASSERT(index < m_ArraySize);
    UniformBuffer::Update(&data, sizeof(T), index * sizeof(T));
  }

}
