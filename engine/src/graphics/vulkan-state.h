#pragma once

#include <vector>
#include <string_view>
#include <unordered_map>
#include <string>
#include <engine/utils/singleton.h>
#include <engine/enums.h>
#include <engine/types.h>

#include "common.h"
#include "vulkan-device.h"
#include "vulkan-swapchain.h"
#include "vulkan-commands.h"

#define MAU_GPU_ZONE(cmd, name)                                                \
  TracyVkZone(VulkanState::Ref().GetTracyCtx(), cmd, name)
#define MAU_GPU_COLLECT(cmd)                                                   \
  TracyVkCollect(VulkanState::Ref().GetTracyCtx(), cmd)

namespace mau {

  class VulkanState: public Singleton<VulkanState> {
    friend class Singleton<VulkanState>;

  private:
    VulkanState(bool enable_validation = false);
    ~VulkanState();

  public:
    void Init(std::string_view app_name, void *window);
    bool EnableInstanceLayer(std::string_view layer_name) noexcept;
    bool EnableInstanceExtension(std::string_view extension_name) noexcept;
    void SetValidationSeverity(VulkanValidationLogSeverity severity,
                               bool                        enabled) noexcept;
    void SetValidationSeverity(TUint32 flags) noexcept;

    Handle<CommandPool> GetCommandPool(VkQueueFlagBits queue_type);
    inline VmaAllocator GetVulkanMemoryAllocator() const { return m_Allocator; }
    inline VkInstance   GetInstance() const { return m_Instance; }

    inline VkDevice GetDevice() const { return m_Device->GetDevice(); }
    inline Handle<VulkanDevice> GetDeviceHandle() const { return m_Device; }
    inline VkPhysicalDevice     GetPhysicalDevice() const {
      return m_PhysicalDevice;
    }

    inline VkSwapchainKHR GetSwapchain() const {
      return m_Swapchain->GetSwapchain();
    }
    inline Handle<VulkanSwapchain> GetSwapchainHandle() const {
      return m_Swapchain;
    }
    inline VkFormat GetSwapchainColorFormat() const {
      return m_Swapchain->GetColorFormat();
    }
    inline VkFormat GetSwapchainDepthFormat() const {
      return m_Swapchain->GetDepthFormat();
    }
    inline VkExtent2D GetSwapchainExtent() const {
      return m_Swapchain->GetExtent();
    }
    inline const std::vector<Handle<ImageView>> GetSwapchainImageViews() const {
      return m_Swapchain->GetImageViews();
    }
    inline const std::vector<Handle<ImageView>>
    GetSwapchainDepthImageViews() const {
      return m_Swapchain->GetDepthImageViews();
    }
    inline VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const {
      return m_PhysicalDeviceProperties;
    }

    inline VkPhysicalDeviceRayTracingPipelinePropertiesKHR
    GetRTPipelineProperties() const {
      return m_RTPipelineProperties;
    }

    inline TracyVkCtx GetTracyCtx() const { return m_TracyContext; }

  private:
    void PickPhysicalDevice();
    bool CreateCommandPool(VkQueueFlagBits queue_type);
    void CreateVulkanMemoryAllocator();
    void InitTracy();
    void ShutdownTracy();

  private:
    bool    m_Validation = false;
    TUint32 m_ValidationSeverity = VulkanValidationLogSeverity::ALL;
    TUint32 m_SelectedPhysicalDeviceIndex = 0u;

    std::vector<const char *> m_InstanceExtensions;
    std::vector<const char *> m_InstanceLayers;

    std::vector<VkLayerProperties>          m_AvailableInstanceLayers;
    std::vector<VkExtensionProperties>      m_AvailableInstanceExtensions;
    std::vector<VkPhysicalDeviceProperties> m_AvailablePhysicalDevices;

    // vulkan handles
    VkInstance                 m_Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT   m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR               m_Surface = VK_NULL_HANDLE;
    VkPhysicalDevice           m_PhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties = {};

    // extension properties
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RTPipelineProperties = {};

    // custom wrappers
    Handle<VulkanDevice>    m_Device = nullptr;
    Handle<VulkanSwapchain> m_Swapchain = nullptr;

    // vulkan memeory allocator
    VmaAllocator m_Allocator = VK_NULL_HANDLE;

    // command pools
    std::unordered_map<VkQueueFlagBits, Handle<CommandPool>> m_CommandPools =
        {};

    // tracy profiler context
    TracyVkCtx            m_TracyContext = nullptr;
    Handle<CommandBuffer> m_TracyCmdBuf = nullptr;
  };

} // namespace mau
