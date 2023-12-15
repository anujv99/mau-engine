#include "vulkan-swapchain.h"

#include <engine/assert.h>

namespace mau {

  VulkanSwapchain::VulkanSwapchain(VkDevice         device,
                                   VkPhysicalDevice physical_device,
                                   VkSurfaceKHR     surface)
      : m_Device(device), m_PhysicalDevice(physical_device),
        m_Surface(surface) {
    ASSERT(m_Device != VK_NULL_HANDLE);
    ASSERT(m_PhysicalDevice != VK_NULL_HANDLE);
    ASSERT(surface != VK_NULL_HANDLE);

    CreateSwapchain();
  }

  VulkanSwapchain::~VulkanSwapchain() { DestroySwapchain(); }

  TUint32 VulkanSwapchain::GetNextImageIndex(Handle<Semaphore> signal) {
    TUint32 image_index = 0u;

    VkResult result =
        vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, signal->Get(),
                              VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      vkDeviceWaitIdle(m_Device);
      DestroySwapchain();
      CreateSwapchain();
      VK_CALL(vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX,
                                    signal->Get(), VK_NULL_HANDLE,
                                    &image_index));
    } else {
      VK_CALL(result);
    }
    return image_index;
  }

  void VulkanSwapchain::RegisterSwapchainCreateCallbackFunc(
      std::function<void(void)> func) {
    m_SwapchainCreateCallback.push_back(func);
  }

  void VulkanSwapchain::CreateSwapchain() {
    // get surface capabilities, formtas, present modes
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_PhysicalDevice, m_Surface, &m_SurfaceCapabilities));

    uint32_t surface_format_count = 0u;
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surface_format_count, nullptr));
    m_SurfaceFormats.resize(static_cast<size_t>(surface_format_count));
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface,
                                                 &surface_format_count,
                                                 m_SurfaceFormats.data()));

    uint32_t surface_present_mode_count = 0u;
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_PhysicalDevice, m_Surface, &surface_present_mode_count, nullptr));
    m_PresentModes.resize(static_cast<size_t>(surface_present_mode_count));
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_PhysicalDevice, m_Surface, &surface_present_mode_count,
        m_PresentModes.data()));

    if (m_SurfaceFormats.size() == 0u) {
      throw GraphicsException("cannot find any surface format on device");
    }

    // pick extent, surface format, present mode, image count
    m_Extent = m_SurfaceCapabilities.minImageExtent;
    m_Format = m_SurfaceFormats[0]; // TODO: just pick the first one for now

    m_PresentMode = VK_PRESENT_MODE_FIFO_KHR; // guarenteed to be supported
    for (const auto &present_mode : m_PresentModes) {
      if (present_mode == m_PrefferedPresentMode)
        m_PresentMode = present_mode; // pick preffered one if available
    }

    m_ImageCount = m_SurfaceCapabilities.minImageCount + 1;
    if (m_SurfaceCapabilities.maxImageCount > 0 &&
        m_ImageCount > m_SurfaceCapabilities.maxImageCount) {
      m_ImageCount = m_SurfaceCapabilities.maxImageCount;
    }

    // create the swapchin
    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0u;
    create_info.surface = m_Surface;
    create_info.minImageCount = m_ImageCount;
    create_info.imageFormat = m_Format.format;
    create_info.imageColorSpace = m_Format.colorSpace;
    create_info.imageExtent = m_Extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.preTransform = m_SurfaceCapabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = m_PresentMode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VK_CALL_REASON(
        vkCreateSwapchainKHR(m_Device, &create_info, nullptr, &m_Swapchain),
        "failed to create swapchain");

    // get swapchain images
    uint32_t swapchain_image_count = 0u;
    VK_CALL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain,
                                    &swapchain_image_count, nullptr));
    m_SwapchainImages.reserve(static_cast<size_t>(swapchain_image_count));
    std::vector<VkImage> swapchain_images(
        static_cast<size_t>(swapchain_image_count));
    VK_CALL(vkGetSwapchainImagesKHR(m_Device, m_Swapchain,
                                    &swapchain_image_count,
                                    swapchain_images.data()));

    for (const auto &image : swapchain_images) {
      m_SwapchainImages.push_back(
          make_handle<Image>(m_Extent.width, m_Extent.height, image,
                             m_Format.format, VK_SAMPLE_COUNT_1_BIT));
    }

    // create image views
    m_SwapchainImageViews.reserve(m_SwapchainImages.size());
    for (const auto &image : m_SwapchainImages) {
      m_SwapchainImageViews.push_back(make_handle<ImageView>(
          image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT));
    }

    // create depth images
    m_DepthFormat = GetDepthFormat(VK_IMAGE_TILING_OPTIMAL);
    m_DepthImages.reserve(m_SwapchainImages.size());
    m_DepthImageViews.reserve(m_SwapchainImages.size());
    for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
      Handle<Image> depth_image = make_handle<Image>(
          m_Extent.width, m_Extent.height, 1, 1, 1, VK_IMAGE_TYPE_2D,
          VK_SAMPLE_COUNT_1_BIT, m_DepthFormat, VK_IMAGE_TILING_OPTIMAL,
          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
      Handle<ImageView> depth_image_view = make_handle<ImageView>(
          depth_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
      m_DepthImages.push_back(depth_image);
      m_DepthImageViews.push_back(depth_image_view);
    }

    // execute callbacks
    for (const auto &callback : m_SwapchainCreateCallback) {
      callback();
    }

    LOG_INFO("vulkan swachain created");
  }

  VkFormat VulkanSwapchain::GetDepthFormat(VkImageTiling tiling) {
    const VkFormat             candidates[] = {VK_FORMAT_D32_SFLOAT,
                                               VK_FORMAT_D32_SFLOAT_S8_UINT,
                                               VK_FORMAT_D24_UNORM_S8_UINT};
    const VkFormatFeatureFlags features =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (size_t i = 0; i < ARRAY_SIZE(candidates); i++) {
      const VkFormat     format = candidates[i];
      VkFormatProperties props = {};
      vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR &&
          (props.linearTilingFeatures & features) == features)
        return format;
      if (tiling == VK_IMAGE_TILING_OPTIMAL &&
          (props.optimalTilingFeatures & features) == features)
        return format;
    }

    return candidates[0];
  }

  void VulkanSwapchain::DestroySwapchain() {
    m_DepthImageViews.clear();
    m_DepthImages.clear();
    m_SwapchainImageViews.clear();
    m_SwapchainImages.clear();
    m_SurfaceFormats.clear();
    m_PresentModes.clear();

    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    m_Swapchain = VK_NULL_HANDLE;
  }

} // namespace mau
