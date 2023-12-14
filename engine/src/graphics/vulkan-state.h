#pragma once

#include <vector>
#include <string_view>
#include <engine/utils/singleton.h>

#include <vulkan/vulkan.h>

namespace mau {

  class VulkanState: public Singleton<VulkanState> {
    friend class Singleton<VulkanState>;
  private:
    VulkanState(bool enable_validation = false);
    ~VulkanState();
  public:
    void Init(std::string_view app_name, const void* window);
    bool EnableInstanceLayer(std::string_view layer_name) noexcept;
    bool EnableInstanceExtension(std::string_view extension_name) noexcept;
  private:
  private:
    bool m_Validation      = false;

    std::vector<std::string_view> m_InstanceExtensions;
    std::vector<std::string_view> m_InstanceLayers;

    std::vector<VkLayerProperties>     m_AvailableInstanceLayers;
    std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;

    // vulkan handles
    VkInstance               m_Instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
  };

}
