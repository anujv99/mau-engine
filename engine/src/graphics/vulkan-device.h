#pragma once

#include <vector>
#include <string_view>
#include <engine/enums.h>
#include <engine/types.h>

#include "common.h"

namespace mau {

  class VulkanDevice {
  public:
    VulkanDevice(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    ~VulkanDevice();
  public:
    bool EnableDeviceExtension(std::string_view extension_name) noexcept;

    inline VkDevice getDevice() const noexcept { return m_Device; }
  private:
    VkPhysicalDevice         m_PhysicalDevice         = VK_NULL_HANDLE;
    VkSurfaceKHR             m_Surface                = VK_NULL_HANDLE;
    VkDevice                 m_Device                 = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures m_EnabledDeviceFeatures  = {};
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures = {};

    std::vector<VkLayerProperties>     m_AvailableDeviceLayers     = {};
    std::vector<VkExtensionProperties> m_AvailableDeviceExtensions = {};

    std::vector<const char*> m_DeviceLayers     = {};
    std::vector<const char*> m_DeviceExtensions = {};

    TUint32 m_GraphicsQueueIndex = UINT32_MAX;
    TUint32 m_TransferQueueIndex = UINT32_MAX;
    TUint32 m_PresentQueueIndex  = UINT32_MAX;

    VkQueue m_GraphicsQueue      = VK_NULL_HANDLE;
    VkQueue m_TransferQueue      = VK_NULL_HANDLE;
    VkQueue m_PresentQueue       = VK_NULL_HANDLE;
  };

}
