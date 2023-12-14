#include "vulkan-device.h"

#include <set>
#include <engine/assert.h>

namespace mau {

  bool hasQueueFamily(VkQueueFamilyProperties properties, VkQueueFlagBits queue) {
    return properties.queueFlags & queue;
  }

  VulkanDevice::VulkanDevice(VkPhysicalDevice physical_device, VkSurfaceKHR surface): m_PhysicalDevice(physical_device), m_Surface(surface) {
    ASSERT(m_PhysicalDevice != VK_NULL_HANDLE);
    ASSERT(m_Surface != VK_NULL_HANDLE);

    // get features
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
    memset(&m_EnabledDeviceFeatures, 0, sizeof(m_EnabledDeviceFeatures));

    m_EnabledDeviceFeatures.samplerAnisotropy = VK_TRUE;

    // enable descriptor indexing
    // TODO: check before enabling
    VkPhysicalDeviceVulkan12Features vulkan12_features              = {};
    vulkan12_features.sType                                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12_features.pNext                                         = nullptr;
    vulkan12_features.runtimeDescriptorArray                        = VK_TRUE;
    vulkan12_features.descriptorIndexing                            = VK_TRUE;
    vulkan12_features.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
    vulkan12_features.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
    vulkan12_features.shaderUniformBufferArrayNonUniformIndexing    = VK_TRUE;
    vulkan12_features.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    vulkan12_features.shaderStorageBufferArrayNonUniformIndexing    = VK_TRUE;
    vulkan12_features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    vulkan12_features.descriptorBindingPartiallyBound               = VK_TRUE;

    VkPhysicalDeviceFeatures2 physical_device_features_2 = {};
    physical_device_features_2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physical_device_features_2.pNext                     = &vulkan12_features;
    physical_device_features_2.features                  = m_EnabledDeviceFeatures;

    // get layers
    uint32_t physical_device_layer_count = 0u;
    VK_CALL(vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &physical_device_layer_count, nullptr));
    m_AvailableDeviceLayers.resize(static_cast<size_t>(physical_device_layer_count));
    VK_CALL(vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &physical_device_layer_count, m_AvailableDeviceLayers.data()));

    // get extensions
    uint32_t physical_device_extension_count = 0u;
    VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &physical_device_extension_count, nullptr));
    m_AvailableDeviceExtensions.resize(static_cast<size_t>(physical_device_extension_count));
    VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &physical_device_extension_count, m_AvailableDeviceExtensions.data()));

    // get queue properties
    uint32_t queue_family_property_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_property_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(static_cast<size_t>(queue_family_property_count));
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_property_count, queue_families.data());

    // find all queue indices
    for (size_t i = 0; i < queue_families.size(); i++) {
      m_GraphicsQueueIndex = m_GraphicsQueueIndex == UINT32_MAX && hasQueueFamily(queue_families[i], VK_QUEUE_GRAPHICS_BIT) ? static_cast<TUint32>(i) : m_GraphicsQueueIndex;
      m_TransferQueueIndex = (m_TransferQueueIndex == UINT32_MAX || m_TransferQueueIndex == m_GraphicsQueueIndex) && hasQueueFamily(queue_families[i], VK_QUEUE_TRANSFER_BIT) ? static_cast<TUint32>(i) : m_TransferQueueIndex;

      VkBool32 present_support = VK_FALSE;
      VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &present_support));
      m_PresentQueueIndex = m_PresentQueueIndex == UINT32_MAX && present_support == VK_TRUE ? static_cast<TUint32>(i) : m_PresentQueueIndex;
    }

    // create devcice and queues
    if (!EnableDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
      throw GraphicsException("failed to enable swapchain extension");
    }

    std::set<TUint32> queue_indices = { m_GraphicsQueueIndex, m_TransferQueueIndex, m_PresentQueueIndex };

    std::vector<VkDeviceQueueCreateInfo> queue_create_info(queue_indices.size());
    float queue_priority = 1.0f;
    for (size_t i = 0; i < queue_indices.size(); i++) {
      auto it = queue_indices.begin();
      std::advance(it, i);

      VkDeviceQueueCreateInfo create_info = {};
      create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      create_info.pNext                   = nullptr;
      create_info.flags                   = 0u;
      create_info.queueFamilyIndex        = *it;
      create_info.queueCount              = 1u;
      create_info.pQueuePriorities        = &queue_priority;

      queue_create_info[i] = create_info;
    }

    VkDeviceCreateInfo device_create_info      = {};
    device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext                   = &physical_device_features_2;
    device_create_info.flags                   = 0u;
    device_create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_info.size());
    device_create_info.pQueueCreateInfos       = queue_create_info.data();
    device_create_info.enabledLayerCount       = static_cast<uint32_t>(m_DeviceLayers.size());
    device_create_info.ppEnabledLayerNames     = m_DeviceLayers.data();
    device_create_info.enabledExtensionCount   = static_cast<uint32_t>(m_DeviceExtensions.size());
    device_create_info.ppEnabledExtensionNames = m_DeviceExtensions.data();
    device_create_info.pEnabledFeatures        = nullptr;

    VK_CALL_REASON(vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &m_Device), "failed to create logical device");

    // get queue handles
    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue transfer_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_Device, m_GraphicsQueueIndex, 0, &graphics_queue);
    vkGetDeviceQueue(m_Device, m_TransferQueueIndex, 0, &transfer_queue);
    vkGetDeviceQueue(m_Device, m_PresentQueueIndex, 0, &present_queue);

    m_GraphicsQueue = make_handle<VulkanQueue>(graphics_queue, m_Device);
    m_TransferQueue = make_handle<VulkanQueue>(transfer_queue, m_Device);
    m_PresentQueue = make_handle<PresentQueue>(present_queue, m_Device);

    LOG_INFO("vulkan logical device created");
  }

  VulkanDevice::~VulkanDevice() {
    vkDestroyDevice(m_Device, nullptr);
  }

  bool VulkanDevice::EnableDeviceExtension(std::string_view extension_name) noexcept {
    if (m_Device != VK_NULL_HANDLE) {
      LOG_ERROR("cannot enable device extension [%s]: device already created", extension_name.data());
      return false;
    }

    bool found = false;
    for (const auto& extension : m_AvailableDeviceExtensions) {
      if (extension.extensionName == extension_name) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_ERROR("cannot enable device extension [%s]: extension not found", extension_name.data());
      return false;
    }

    m_DeviceExtensions.push_back(extension_name.data());
    LOG_INFO("enabled device extension: %s", extension_name.data());
    return true;
  }

}
