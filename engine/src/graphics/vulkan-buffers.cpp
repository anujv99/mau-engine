#include "vulkan-buffers.h"

#include "vulkan-state.h"
#include "vulkan-features.h"

namespace mau {

  void UploadUsingStaging(Buffer* dst, const void* data) {
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

    Handle<VulkanQueue> transfer_queue = VulkanState::Ref().GetDeviceHandle()->GetTransferQueue();

    transfer_queue->Submit(cmd);
    transfer_queue->WaitIdle();
  }

  Buffer::Buffer(TUint64 buffer_size, VkBufferUsageFlags usage, VmaAllocationCreateFlags memory_flags): m_Size(buffer_size) {
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.usage                   = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags                   = memory_flags;

    if (VulkanFeatures::IsBufferDeviceAddressEnabled()) {
      usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

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
    if (m_MappedMemory) UnMap();
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

  VkDeviceAddress Buffer::GetDeviceAddress() {
    if (!VulkanFeatures::IsBufferDeviceAddressEnabled()) {
      LOG_ERROR("calling get buffer device address when feature is not enabled");
      return m_DeviceAddress;
    }

    if (m_DeviceAddress == 0ui64) {
      VkBufferDeviceAddressInfo address_info = {};
      address_info.sType                     = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
      address_info.pNext                     = nullptr;
      address_info.buffer                    = m_Buffer;

      m_DeviceAddress = vkGetBufferDeviceAddress(VulkanState::Ref().GetDevice(), &address_info);
    }

    return m_DeviceAddress;
  }

  VertexBuffer::VertexBuffer(TUint64 buffer_size, const void* data):
    Buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) {
    
    if (data) {
      UploadUsingStaging(this, data);
    }
  }

  VertexBuffer::~VertexBuffer() { }

  IndexBuffer::IndexBuffer(TUint64 buffer_size, const void* data):
    Buffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) {

    if (data) {
      UploadUsingStaging(this, data);
    }
  }

  IndexBuffer::~IndexBuffer() { }

  UniformBuffer::UniformBuffer(TUint64 buffer_size, const void* data):
    Buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT) {

    Map();
    if (data) {
      memcpy(m_MappedMemory, data, buffer_size);
    }
  }

  UniformBuffer::~UniformBuffer() { }

  void UniformBuffer::Update(const void* data, TUint64 size, TUint64 offset) {
    if (m_MappedMemory) {
      char* dst = reinterpret_cast<char*>(m_MappedMemory);
      memcpy(reinterpret_cast<void*>(dst + offset), data, size);
      m_IsUpdated = true;
    }
  }

  void UniformBuffer::Flush(Handle<CommandBuffer> cmd) {
    if (m_IsUpdated) {
      m_IsUpdated = false;
      // TODO: add pipeline barrier
    }
  }

  VkDescriptorBufferInfo UniformBuffer::GetDescriptorInfo() const {
    VkDescriptorBufferInfo descriptor_info = {};
    descriptor_info.buffer                 = m_Buffer;
    descriptor_info.offset                 = 0ui64;
    descriptor_info.range                  = m_Size;

    return descriptor_info;
  }

  // acceleration structure for ray tracing

  bool validate_acceleration_buffer_create_info(const AccelerationBufferCreateInfo& create_info) {
    return create_info.Vertices && create_info.Indices;
  }

  AccelerationBuffer::AccelerationBuffer(const AccelerationBufferCreateInfo& create_info) {
    ASSERT(validate_acceleration_buffer_create_info(create_info));

    // build bottom level accel
    BuildBLAS(create_info);

    // build top level accel
    BuildTLAS();
  }

  AccelerationBuffer::~AccelerationBuffer() {
    vkDestroyAccelerationStructureKHR(VulkanState::Ref().GetDevice(), m_BLAS, nullptr);
    vkDestroyAccelerationStructureKHR(VulkanState::Ref().GetDevice(), m_TLAS, nullptr);
    m_BLASBuffer = nullptr;
    m_TLASBuffer = nullptr;
  }

  void AccelerationBuffer::BuildBLAS(const AccelerationBufferCreateInfo& create_info) {
    const TUint32 max_primitive_count = create_info.IndexCount / 3u;

    VkDeviceOrHostAddressConstKHR vertex_buffer_address = {
      .deviceAddress = create_info.Vertices->GetDeviceAddress(),
    };

    VkDeviceOrHostAddressConstKHR index_buffer_address = {
      .deviceAddress = create_info.Indices->GetDeviceAddress(),
    };

    VkAccelerationStructureGeometryTrianglesDataKHR triangles = {
      .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
      .pNext         = nullptr,
      .vertexFormat  = VK_FORMAT_R32G32B32_SFLOAT,
      .vertexData    = vertex_buffer_address,
      .vertexStride  = create_info.VertexSize,
      .maxVertex     = create_info.VertexCount,
      .indexType     = VK_INDEX_TYPE_UINT32,
      .indexData     = index_buffer_address,
      .transformData = 0ui64,
    };

    VkAccelerationStructureGeometryDataKHR geometry_data = {
      .triangles = triangles,
    };

    VkAccelerationStructureGeometryKHR geometry = {
      .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
      .pNext        = nullptr,
      .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
      .geometry     = geometry_data,
      .flags        = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };

    VkAccelerationStructureBuildRangeInfoKHR offset = {
      .primitiveCount  = max_primitive_count,
      .primitiveOffset = 0u,
      .firstVertex     = 0u,
      .transformOffset = 0u,
    };

    // build bottom level accel structure
    VkAccelerationStructureBuildGeometryInfoKHR blas_build_info = {
      .sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext                    = nullptr,
      .type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .flags                    = 0u,
      .mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount            = 1u,
      .pGeometries              = &geometry,
      .ppGeometries             = nullptr,
      .scratchData              = { .hostAddress = nullptr },
    };

    VkAccelerationStructureBuildSizesInfoKHR blas_size_info = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };

    vkGetAccelerationStructureBuildSizesKHR(VulkanState::Ref().GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &blas_build_info, &max_primitive_count, &blas_size_info);

    Buffer scratch_buffer(blas_size_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
    VkDeviceAddress scrach_address = scratch_buffer.GetDeviceAddress();

    m_BLASBuffer = make_handle<Buffer>(blas_size_info.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

    VkAccelerationStructureCreateInfoKHR blas_create_info = {
      .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
      .pNext         = nullptr,
      .createFlags   = 0u,
      .buffer        = m_BLASBuffer->Get(),
      .offset        = 0ui64,
      .size          = blas_size_info.accelerationStructureSize,
      .type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
      .deviceAddress = 0ui64,
    };

    VK_CALL(vkCreateAccelerationStructureKHR(VulkanState::Ref().GetDevice(), &blas_create_info, nullptr, &m_BLAS));

    blas_build_info.dstAccelerationStructure  = m_BLAS;
    blas_build_info.scratchData.deviceAddress = scrach_address;

    VkAccelerationStructureBuildRangeInfoKHR* range_info[] = { &offset };

    Handle<CommandBuffer> cmd = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT)->AllocateCommandBuffers(1)[0];
    cmd->Begin();
    vkCmdBuildAccelerationStructuresKHR(cmd->Get(), 1u, &blas_build_info, range_info);
    cmd->End();

    Handle<VulkanQueue> graphics_queue = VulkanState::Ref().GetDeviceHandle()->GetGraphicsQueue();
    graphics_queue->Submit(cmd);
    graphics_queue->WaitIdle();
  }

  void AccelerationBuffer::BuildTLAS(Handle<CommandBuffer> in_cmd, bool update, glm::mat4 transform) {
    const TUint32 max_primitive_count = 1u;

    VkTransformMatrixKHR transform_matrix = {
      transform[0][0], transform[1][0], transform[2][0], transform[3][0],
      transform[0][1], transform[1][1], transform[2][1], transform[3][1],
      transform[0][2], transform[1][2], transform[2][2], transform[3][2],
    };

    VkAccelerationStructureDeviceAddressInfoKHR blas_address_info = {
      .sType                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
      .pNext                 = nullptr,
      .accelerationStructure = m_BLAS,
    };
    VkDeviceAddress blas_address = vkGetAccelerationStructureDeviceAddressKHR(VulkanState::Ref().GetDevice(), &blas_address_info);

    VkAccelerationStructureInstanceKHR tlas_instance = {
      .transform                              = transform_matrix,
      .instanceCustomIndex                    = 0u,
      .mask                                   = 0xff,
      .instanceShaderBindingTableRecordOffset = 0u,
      .flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
      .accelerationStructureReference         = blas_address,
    };

    if (m_InstanceBuffer == nullptr) {
      m_InstanceBuffer = make_handle<Buffer>(sizeof(tlas_instance), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    }

    void* instance_buffer_mem = m_InstanceBuffer->Map();
    memcpy(instance_buffer_mem, &tlas_instance, sizeof(tlas_instance));
    m_InstanceBuffer->UnMap();

    VkDeviceOrHostAddressConstKHR instance_buffer_address = {
      .deviceAddress = m_InstanceBuffer->GetDeviceAddress(),
    };

    VkAccelerationStructureGeometryInstancesDataKHR instances = {
      .sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
      .pNext           = nullptr,
      .arrayOfPointers = VK_FALSE,
      .data            = instance_buffer_address,
    };

    VkAccelerationStructureGeometryDataKHR geometry_data = {
      .instances = instances,
    };

    VkAccelerationStructureGeometryKHR geometry = {
      .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
      .pNext        = nullptr,
      .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
      .geometry     = geometry_data,
      .flags        = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };

    VkAccelerationStructureBuildGeometryInfoKHR tlas_build_info = {
      .sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
      .pNext                    = nullptr,
      .type                     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
      .flags                    = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
      .mode                     = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
      .srcAccelerationStructure = VK_NULL_HANDLE,
      .dstAccelerationStructure = VK_NULL_HANDLE,
      .geometryCount            = 1u,
      .pGeometries              = &geometry,
      .ppGeometries             = nullptr,
      .scratchData              = { .hostAddress = nullptr },
    };

    VkAccelerationStructureBuildSizesInfoKHR tlas_size_info = {
      .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
    };

    vkGetAccelerationStructureBuildSizesKHR(VulkanState::Ref().GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &tlas_build_info, &max_primitive_count, &tlas_size_info);

    if (m_TLASScratchBuffer == nullptr || m_TLASScratchBuffer->GetSize() != tlas_size_info.buildScratchSize) {
      m_TLASScratchBuffer = make_handle<Buffer>(tlas_size_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
    }

    VkDeviceAddress scrach_address = m_TLASScratchBuffer->GetDeviceAddress();

    if (update == false) {
      m_TLASBuffer = make_handle<Buffer>(tlas_size_info.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);
      VkAccelerationStructureCreateInfoKHR tlas_create_info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = 0u,
        .buffer = m_TLASBuffer->Get(),
        .offset = 0ui64,
        .size = tlas_size_info.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .deviceAddress = 0ui64,
      };

      VK_CALL(vkCreateAccelerationStructureKHR(VulkanState::Ref().GetDevice(), &tlas_create_info, nullptr, &m_TLAS));
    }

    tlas_build_info.srcAccelerationStructure  = update ? m_TLAS : VK_NULL_HANDLE;
    tlas_build_info.dstAccelerationStructure  = m_TLAS;
    tlas_build_info.scratchData.deviceAddress = scrach_address;

    VkAccelerationStructureBuildRangeInfoKHR build_offset = { 1u, 0u, 0u, 0u };
    VkAccelerationStructureBuildRangeInfoKHR* range_info[] = { &build_offset };

    Handle<CommandBuffer> cmd = in_cmd ? in_cmd : VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT)->AllocateCommandBuffers(1)[0];

    if (in_cmd == nullptr) {
      cmd->Begin();
      vkCmdBuildAccelerationStructuresKHR(cmd->Get(), 1u, &tlas_build_info, range_info);
      cmd->End();

      Handle<VulkanQueue> graphics_queue = VulkanState::Ref().GetDeviceHandle()->GetGraphicsQueue();
      graphics_queue->Submit(cmd);
      graphics_queue->WaitIdle();
    } else {
      vkCmdBuildAccelerationStructuresKHR(cmd->Get(), 1u, &tlas_build_info, range_info);
    }
  }

  void AccelerationBuffer::UpdateTransform(const glm::mat4& transform, Handle<CommandBuffer> cmd) {
    BuildTLAS(cmd, true, transform);
  }

}
