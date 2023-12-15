#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <tracy/TracyVulkan.hpp>

#include <engine/exceptions.h>
#include <engine/log.h>
#include <engine/utils/handle.h>

#define VMA_VULKAN_VERSION 1003000
#include <vk_mem_alloc.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define VK_CALL(call)                                                                                                                                                                                  \
  {                                                                                                                                                                                                    \
    VkResult ret = call;                                                                                                                                                                               \
    if (ret != VK_SUCCESS) {                                                                                                                                                                           \
      LOG_ERROR(#call " failed: %s", string_VkResult(ret));                                                                                                                                            \
      throw GraphicsException(#call "failed");                                                                                                                                                         \
    }                                                                                                                                                                                                  \
  }

#define VK_CALL_REASON(call, reason)                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    VkResult ret = call;                                                                                                                                                                               \
    if (ret != VK_SUCCESS) {                                                                                                                                                                           \
      LOG_ERROR(#call " failed: %s", string_VkResult(ret));                                                                                                                                            \
      throw GraphicsException(reason);                                                                                                                                                                 \
    }                                                                                                                                                                                                  \
  }

namespace mau {

  extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
  extern PFN_vkGetAccelerationStructureBuildSizesKHR    vkGetAccelerationStructureBuildSizesKHR;
  extern PFN_vkCreateAccelerationStructureKHR           vkCreateAccelerationStructureKHR;
  extern PFN_vkDestroyAccelerationStructureKHR          vkDestroyAccelerationStructureKHR;
  extern PFN_vkCmdBuildAccelerationStructuresKHR        vkCmdBuildAccelerationStructuresKHR;
  extern PFN_vkCreateRayTracingPipelinesKHR             vkCreateRayTracingPipelinesKHR;
  extern PFN_vkCmdTraceRaysKHR                          vkCmdTraceRaysKHR;
  extern PFN_vkGetRayTracingShaderGroupHandlesKHR       vkGetRayTracingShaderGroupHandlesKHR;

  using TextureHandle = TUint32;
  using BufferHandle = TUint32;
  using MaterialHandle = TUint32;
  using ImageHandle = TUint32;
  using AccelerationStructureHandle = TUint32;
  using BufferAddress = TUint64;
  using RTObjectHandle = TUint32; // 24 bit only

  struct GPUMaterial {
    TextureHandle Diffuse = UINT32_MAX;
    TextureHandle Normal = UINT32_MAX;

    TUint32 padding[2];
  };

  struct RTObjectDesc {
    BufferAddress  VertexBuffer = UINT64_MAX; // 64 bit
    BufferAddress  IndexBuffer = UINT64_MAX;  // 64 bit
    MaterialHandle Material = UINT32_MAX;     // 32 bit

    TUint32 padding[3];
  };

  template <class integral> constexpr integral align_up(integral x, size_t a) noexcept { return integral((x + (integral(a) - 1)) & ~integral(a - 1)); }

} // namespace mau
