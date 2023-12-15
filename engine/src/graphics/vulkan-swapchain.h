#pragma once

#include <vector>
#include <functional>
#include <engine/types.h>
#include "common.h"
#include "vulkan-image.h"
#include "vulkan-sync.h"

namespace mau {

  class VulkanSwapchain: public HandledObject {
  public:
    VulkanSwapchain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    ~VulkanSwapchain();

  public:
    TUint32 GetNextImageIndex(Handle<Semaphore> signal);
    void    RegisterSwapchainCreateCallbackFunc(std::function<void(void)> func);

  public:
    inline VkSwapchainKHR                        GetSwapchain() const { return m_Swapchain; }
    inline const VkSwapchainKHR                 *Ref() const { return &m_Swapchain; }
    inline VkFormat                              GetColorFormat() const { return m_Format.format; }
    inline VkFormat                              GetDepthFormat() const { return m_DepthFormat; }
    inline VkSurfaceCapabilitiesKHR              GetSurfaceCapabilities() const { return m_SurfaceCapabilities; }
    inline VkExtent2D                            GetExtent() const { return m_Extent; }
    inline const std::vector<Handle<ImageView>> &GetImageViews() const { return m_SwapchainImageViews; }
    inline const std::vector<Handle<ImageView>> &GetDepthImageViews() const { return m_DepthImageViews; }
    inline const std::vector<Handle<Image>>     &GetImages() const { return m_SwapchainImages; }
    inline const std::vector<Handle<Image>>     &GetDepthImages() const { return m_DepthImages; }

  private:
    void     CreateSwapchain();
    VkFormat GetDepthFormat(VkImageTiling tiling);
    void     DestroySwapchain();

  private:
    VkDevice         m_Device = VK_NULL_HANDLE;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR     m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR   m_Swapchain = VK_NULL_HANDLE;

    VkSurfaceCapabilitiesKHR        m_SurfaceCapabilities = {};
    std::vector<VkSurfaceFormatKHR> m_SurfaceFormats = {};
    std::vector<VkPresentModeKHR>   m_PresentModes = {};

    // TODO: make it configurable
    const VkPresentModeKHR m_PrefferedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    TUint32                        m_ImageCount = 0u;
    VkExtent2D                     m_Extent = {};
    VkSurfaceFormatKHR             m_Format = {};
    VkFormat                       m_DepthFormat = VK_FORMAT_UNDEFINED;
    VkPresentModeKHR               m_PresentMode = {};
    std::vector<Handle<Image>>     m_SwapchainImages = {};
    std::vector<Handle<ImageView>> m_SwapchainImageViews = {};
    std::vector<Handle<Image>>     m_DepthImages = {};
    std::vector<Handle<ImageView>> m_DepthImageViews = {};

    // TODO: find something better
    std::vector<std::function<void(void)>> m_SwapchainCreateCallback = {};
  };

} // namespace mau
