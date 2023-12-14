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

#define VK_CALL(call) {                                   \
  VkResult ret = call;                                    \
  if (ret != VK_SUCCESS) {                                \
    LOG_ERROR(#call " failed: %s", string_VkResult(ret)); \
    throw GraphicsException(#call "failed");              \
  }                                                       \
}

#define VK_CALL_REASON(call, reason) {                    \
  VkResult ret = call;                                    \
  if (ret != VK_SUCCESS) {                                \
    LOG_ERROR(#call " failed: %s", string_VkResult(ret)); \
    throw GraphicsException(reason);                      \
  }                                                       \
}

namespace mau {

  using TextureHandle  = TUint32;
  using BufferHandle   = TUint32;
  using MaterialHandle = TUint32;

  struct GPUMaterial {
    TextureHandle Diffuse = UINT32_MAX;
    TextureHandle Normal  = UINT32_MAX;

    float padding[2];
  };

}
