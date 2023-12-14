#pragma once

#include <vector>
#include <engine/types.h>
#include <memory>
#include "common.h"
#include "vulkan-image.h"

namespace mau {

  class VulkanSwapchain {
  public:
    VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    ~VulkanSwapchain();
  private:
    void CreateSwapchain();
    void DestroySwapchain();
  private:
    VkDevice         m_Device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR     m_Surface        = VK_NULL_HANDLE;
    VkSwapchainKHR   m_Swapchain      = VK_NULL_HANDLE;

    VkSurfaceCapabilitiesKHR        m_SurfaceCapabilities = {};
    std::vector<VkSurfaceFormatKHR> m_SurfaceFormats      = {};
    std::vector<VkPresentModeKHR>   m_PresentModes        = {};

    // TODO: make it configurable
    const VkPresentModeKHR m_PrefferedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    TUint32                                 m_ImageCount          = 0u;
    VkExtent2D                              m_Extent              = {};
    VkSurfaceFormatKHR                      m_Format              = {};
    VkPresentModeKHR                        m_PresentMode         = {};
    std::vector<VkImage>                    m_SwapchainImages     = {};
    std::vector<std::shared_ptr<ImageView>> m_SwapchainImageViews = {};
  };

}
