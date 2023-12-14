#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <engine/exceptions.h>

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
