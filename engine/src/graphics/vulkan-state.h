#pragma once

#include <vector>
#include <string_view>
#include <engine/utils/singleton.h>
#include <engine/enums.h>
#include <engine/types.h>

#include <vulkan/vulkan.h>

namespace mau {

  class VulkanState: public Singleton<VulkanState> {
    friend class Singleton<VulkanState>;
  private:
    VulkanState(bool enable_validation = false);
    ~VulkanState();
  public:
    void Init(std::string_view app_name, void* window);
    bool EnableInstanceLayer(std::string_view layer_name) noexcept;
    bool EnableInstanceExtension(std::string_view extension_name) noexcept;
    void SetValidationSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept;
    void SetValidationSeverity(TUint32 flags) noexcept;
  private:
    bool    m_Validation         = false;
    TUint32 m_ValidationSeverity = VulkanValidationLogSeverity::ALL;

    std::vector<const char*> m_InstanceExtensions;
    std::vector<const char*> m_InstanceLayers;

    std::vector<VkLayerProperties>     m_AvailableInstanceLayers;
    std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;

    // vulkan handles
    VkInstance               m_Instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR             m_Surface        = VK_NULL_HANDLE;
  };

}
