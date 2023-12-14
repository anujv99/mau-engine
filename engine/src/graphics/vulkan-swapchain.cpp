#include "vulkan-swapchain.h"

#include <engine/assert.h>

namespace mau {

  VulkanSwapchain::VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface):
    m_Device(device), m_PhysicalDevice(physical_device), m_Surface(surface) {
    ASSERT(m_Device != VK_NULL_HANDLE);
    ASSERT(m_PhysicalDevice != VK_NULL_HANDLE);
    ASSERT(surface != VK_NULL_HANDLE);

    CreateSwapchain();
  }

  VulkanSwapchain::~VulkanSwapchain() {
    DestroySwapchain();
  }

  void VulkanSwapchain::CreateSwapchain() {
    // get surface capabilities, formtas, present modes
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_SurfaceCapabilities));

    uint32_t surface_format_count = 0u;
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, nullptr));
    m_SurfaceFormats.resize(static_cast<size_t>(surface_format_count));
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, m_SurfaceFormats.data()));

    uint32_t surface_present_mode_count = 0u;
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &surface_present_mode_count, nullptr));
    m_PresentModes.resize(static_cast<size_t>(surface_present_mode_count));
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &surface_present_mode_count, m_PresentModes.data()));

    if (m_SurfaceFormats.size() == 0ui64) {
      throw GraphicsException("cannot find any surface format on device");
    }

    // pick extent, surface format, present mode, image count
    m_Extent = m_SurfaceCapabilities.minImageExtent;
    m_Format = m_SurfaceFormats[0]; // TODO: just pick the first one for now

    m_PresentMode = VK_PRESENT_MODE_FIFO_KHR; // guarenteed to be supported
    for (const auto& present_mode : m_PresentModes) {
      if (present_mode == m_PrefferedPresentMode) m_PresentMode = present_mode; // pick preffered one if available
    }

    m_ImageCount = m_SurfaceCapabilities.minImageCount + 1;
    if (m_SurfaceCapabilities.maxImageCount > 0 && m_ImageCount > m_SurfaceCapabilities.maxImageCount) {
      m_ImageCount = m_SurfaceCapabilities.maxImageCount;
    }

    // create the swapchin
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext                    = nullptr;
    create_info.flags                    = 0u;
    create_info.surface                  = m_Surface;
    create_info.minImageCount            = m_ImageCount;
    create_info.imageFormat              = m_Format.format;
    create_info.imageColorSpace          = m_Format.colorSpace;
    create_info.imageExtent              = m_Extent;
    create_info.imageArrayLayers         = 1;
    create_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount    = 0;
    create_info.pQueueFamilyIndices      = nullptr;
    create_info.preTransform             = m_SurfaceCapabilities.currentTransform;
    create_info.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode              = m_PresentMode;
    create_info.clipped                  = VK_TRUE;
    create_info.oldSwapchain             = VK_NULL_HANDLE;

    VK_CALL_REASON(vkCreateSwapchainKHR(m_Device, &create_info, nullptr, &m_Swapchain), "failed to create swapchain");

    // get swapchain images
    uint32_t swapchain_image_count = 0u;
    VK_CALL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchain_image_count, nullptr));
    m_SwapchainImages.resize(static_cast<size_t>(swapchain_image_count));
    VK_CALL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchain_image_count, m_SwapchainImages.data()));

    // create image views
    m_SwapchainImageViews.reserve(m_SwapchainImages.size());
    for (const auto& image : m_SwapchainImages) {
      m_SwapchainImageViews.push_back(std::make_shared<ImageView>(image, m_Format.format, VK_IMAGE_VIEW_TYPE_2D, m_Device));
    }

    LOG_INFO("vulkan swachain created");
  }

  void VulkanSwapchain::DestroySwapchain() {
    m_SwapchainImageViews.clear();
    m_SwapchainImages.clear();
    m_SurfaceFormats.clear();
    m_PresentModes.clear();

    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    m_Swapchain = VK_NULL_HANDLE;
  }

}
