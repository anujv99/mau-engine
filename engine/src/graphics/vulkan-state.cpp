#include "vulkan-state.h"

#include <GLFW/glfw3.h>

#include <engine/log.h>
#include <engine/assert.h>

namespace mau {

  PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    TUint32 validation_severity = *(reinterpret_cast<TUint32*>(user_data));

    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      if (validation_severity & VulkanValidationLogSeverity::VERBOSE) {
        LOG_TRACE("%s", callback_data->pMessage);
      }
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      if (validation_severity & VulkanValidationLogSeverity::INFO) {
        LOG_INFO("%s", callback_data->pMessage);
      }
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      if (validation_severity & VulkanValidationLogSeverity::WARNING) {
        LOG_WARN("%s", callback_data->pMessage);
      }
      break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      if (validation_severity & VulkanValidationLogSeverity::ERROR) {
        LOG_ERROR("%s", callback_data->pMessage);
      }
      break;
    default:
      break;
    }

    return VK_FALSE;
  }

  template <typename T>
  T load_instance_function(VkInstance instance, std::string_view name) {
    T func = reinterpret_cast<T>(vkGetInstanceProcAddr(instance, name.data()));

    if (func == nullptr) {
      LOG_ERROR("failed to load vulkan instance function: %s", name.data());
    }

    return func;
  }

  template <typename T>
  T load_device_function(VkDevice device, std::string_view name) {
    T func = reinterpret_cast<T>(vkGetDeviceProcAddr(device, name.data()));

    if (func == nullptr) {
      LOG_ERROR("failed to load vulkan instance function: %s", name.data());
    }

    return func;
  }

  VkDebugUtilsMessengerEXT create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& create_info) {
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    auto vkCreateDebugUtilsMessengerEXT = load_instance_function<PFN_vkCreateDebugUtilsMessengerEXT>(instance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT) {
      VK_CALL(vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger));
      LOG_INFO("vulkan debug messenger created");
    }

    return debug_messenger;
  }

  void destroy_debug_utils(VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
    auto vkDestroyDebugUtilsMessengerEXT = load_instance_function<PFN_vkDestroyDebugUtilsMessengerEXT>(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (vkDestroyDebugUtilsMessengerEXT) {
      vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
    }
  }

  VulkanState::VulkanState(bool enable_validation): m_Validation(enable_validation) {
    // get available instance layers
    uint32_t instance_layer_count = 0;
    VK_CALL(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));
    m_AvailableInstanceLayers.resize(static_cast<size_t>(instance_layer_count));
    VK_CALL(vkEnumerateInstanceLayerProperties(&instance_layer_count, m_AvailableInstanceLayers.data()));

    // get available instance extensions
    uint32_t instance_extension_count = 0;
    VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));
    m_AvailableInstanceExtensions.resize(static_cast<size_t>(instance_extension_count));
    VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, m_AvailableInstanceExtensions.data()));

    // add validation layer if required
    if (m_Validation) {
      EnableInstanceLayer("VK_LAYER_KHRONOS_validation");
      EnableInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // add surface extensions
    uint32_t glfw_extension_count = 0u;
    const char* const* extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (uint32_t i = 0; i < glfw_extension_count; i++) {
      if (!EnableInstanceExtension(extensions[i])) {
        throw GraphicsException("failed to find required instance extension");
      }
    }
  }

  VulkanState::~VulkanState() {
    ShutdownTracy();
    m_CommandPools.clear();
    m_Swapchain = nullptr;
    vmaDestroyAllocator(m_Allocator);
    m_Device = nullptr;
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    if (m_DebugMessenger) destroy_debug_utils(m_Instance, m_DebugMessenger);
    vkDestroyInstance(m_Instance, nullptr);
  }

  void VulkanState::Init(std::string_view app_name, void* window) {
    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext              = NULL;
    app_info.pApplicationName   = app_name.data();
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.pEngineName        = app_name.data();
    app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info    = {};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext                   = nullptr;
    create_info.flags                   = 0u;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledLayerCount       = static_cast<uint32_t>(m_InstanceLayers.size());
    create_info.ppEnabledLayerNames     = reinterpret_cast<const char* const*>(m_InstanceLayers.data());
    create_info.enabledExtensionCount   = static_cast<uint32_t>(m_InstanceExtensions.size());
    create_info.ppEnabledExtensionNames = reinterpret_cast<const char* const*>(m_InstanceExtensions.data());

    // enable validation for instance
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = {};
    if (m_Validation) {
      debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_messenger_create_info.pNext = nullptr;
      debug_messenger_create_info.flags = 0u;
      debug_messenger_create_info.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_messenger_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
      debug_messenger_create_info.pfnUserCallback = debug_callback;
      debug_messenger_create_info.pUserData = reinterpret_cast<void*>(&m_ValidationSeverity);

      create_info.pNext = reinterpret_cast<void*>(&debug_messenger_create_info);
    }

    // create vulkan instance
    VK_CALL_REASON(vkCreateInstance(&create_info, nullptr, &m_Instance), "failed to create vulkan instance");
    LOG_INFO("vulkan instance created");

    // create debug messenger
    if (m_Validation) {
      m_DebugMessenger = create_debug_utils_messenger(m_Instance, debug_messenger_create_info);
    }

    // create window surface
    ASSERT(window != nullptr);
    GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(window);
    VK_CALL_REASON(glfwCreateWindowSurface(m_Instance, glfw_window, nullptr, &m_Surface), "failed to create surface");
    LOG_INFO("vulkan surface created");

    // pick physical device
    PickPhysicalDevice();

    // create device
    m_Device = make_handle<VulkanDevice>(m_PhysicalDevice, m_Surface);

    // load functions
    vkCmdPushDescriptorSetKHR = load_device_function<PFN_vkCmdPushDescriptorSetKHR>(m_Device->GetDevice(), "vkCmdPushDescriptorSetKHR");

    // create memory allocator
    CreateVulkanMemoryAllocator();

    // create swapchain
    m_Swapchain = make_handle<VulkanSwapchain>(m_Device->GetDevice(), m_PhysicalDevice, m_Surface);

    // pre-create command pools
    CreateCommandPool(VK_QUEUE_GRAPHICS_BIT);
    CreateCommandPool(VK_QUEUE_TRANSFER_BIT);

    // init tracy
    InitTracy();
  }

  bool VulkanState::EnableInstanceLayer(std::string_view layer_name) noexcept {
    if (m_Instance != VK_NULL_HANDLE) {
      LOG_ERROR("cannot enable instance layer [%s]: instance already created", layer_name.data());
      return false;
    }

    bool found = false;
    for (const auto& layer : m_AvailableInstanceLayers) {
      if (layer.layerName == layer_name) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_ERROR("cannot enable instance layer [%s]: layer not found", layer_name.data());
      return false;
    }

    m_InstanceLayers.push_back(layer_name.data());
    LOG_INFO("enabled instance layer: %s", layer_name.data());
    return true;
  }

  bool VulkanState::EnableInstanceExtension(std::string_view extension_name) noexcept {
    if (m_Instance != VK_NULL_HANDLE) {
      LOG_ERROR("cannot enable instance extension [%s]: instance already created", extension_name.data());
      return false;
    }

    bool found = false;
    for (const auto& layer : m_AvailableInstanceExtensions) {
      if (layer.extensionName == extension_name) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_ERROR("cannot enable instance extension [%s]: extension not found", extension_name.data());
      return false;
    }

    m_InstanceExtensions.push_back(extension_name.data());
    LOG_INFO("enabled instance extension: %s", extension_name.data());
    return true;
  }

  void VulkanState::SetValidationSeverity(VulkanValidationLogSeverity severity, bool enabled) noexcept {
    m_ValidationSeverity = enabled ? m_ValidationSeverity | severity : m_ValidationSeverity & ~severity;
  }

  void VulkanState::SetValidationSeverity(TUint32 flags) noexcept {
    m_ValidationSeverity = flags;
  }

  Handle<CommandPool> VulkanState::GetCommandPool(VkQueueFlagBits queue_type) {
    if (m_CommandPools.find(queue_type) != m_CommandPools.end()) {
      return m_CommandPools.at(queue_type);
    } else {
      bool result = CreateCommandPool(queue_type);
      if (result) {
        return m_CommandPools.at(queue_type);
      }
    }

    return nullptr;
  }

  void VulkanState::PickPhysicalDevice() {
    uint32_t physical_device_count = 0u;
    VK_CALL(vkEnumeratePhysicalDevices(m_Instance, &physical_device_count, nullptr));
    std::vector<VkPhysicalDevice> available_physical_devices(static_cast<size_t>(physical_device_count));
    m_AvailablePhysicalDevices.reserve(static_cast<size_t>(physical_device_count));
    VK_CALL(vkEnumeratePhysicalDevices(m_Instance, &physical_device_count, available_physical_devices.data()));

    if (physical_device_count == 0u) {
      throw GraphicsException("no physical device found");
    }

    TInt32 selected_physical_device = -1;

    for (size_t i = 0; i < available_physical_devices.size(); i++) {
      VkPhysicalDeviceProperties physical_device_properties = {};
      vkGetPhysicalDeviceProperties(available_physical_devices[i], &physical_device_properties);
      m_AvailablePhysicalDevices.push_back(physical_device_properties);

      // just pick the discrete gpu for now
      if (selected_physical_device == -1 && physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        selected_physical_device = static_cast<TInt32>(i);
      }
    }

    // if not found, pick the first device
    if (selected_physical_device == -1) selected_physical_device = 0;

    m_SelectedPhysicalDeviceIndex = static_cast<TUint32>(selected_physical_device);
    m_PhysicalDevice = available_physical_devices[static_cast<size_t>(selected_physical_device)];

    LOG_INFO("selected physical device: " LOG_COLOR_PINK "%s", m_AvailablePhysicalDevices[static_cast<size_t>(m_SelectedPhysicalDeviceIndex)].deviceName);
  }

  bool VulkanState::CreateCommandPool(VkQueueFlagBits queue_type) {
    TUint32 queue_index = UINT32_MAX;
    TUint32 flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (m_CommandPools.find(queue_type) != m_CommandPools.end()) {
      LOG_WARN("command pool with queue type %s already exists", string_VkQueueFlagBits(queue_type));
      return false;
    }

    switch (queue_type) {
    case VK_QUEUE_GRAPHICS_BIT:
      queue_index = m_Device->GetGraphicsQueueIndex();
      break;
    case VK_QUEUE_TRANSFER_BIT:
      queue_index = m_Device->GetTransferQueueIndex();
      flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      break;
    default:
      LOG_ERROR("cannot create command pool, invalid queue type");
      return false;
      break;
    }

    if (queue_index == UINT32_MAX) {
      LOG_ERROR("cannot create command pool, failed to find queue index");
      return false;
    }

    m_CommandPools[queue_type] = make_handle<CommandPool>(m_Device->GetDevice(), queue_index, flags);
    return true;
  }

  void VulkanState::CreateVulkanMemoryAllocator() {
    VmaAllocatorCreateInfo create_info         = {};
    create_info.flags                          = {};
    create_info.physicalDevice                 = m_PhysicalDevice;
    create_info.device                         = m_Device->GetDevice();
    create_info.preferredLargeHeapBlockSize    = 0ui64;
    create_info.pAllocationCallbacks           = nullptr;
    create_info.pDeviceMemoryCallbacks         = nullptr;
    create_info.pHeapSizeLimit                 = nullptr;
    create_info.pVulkanFunctions               = nullptr;
    create_info.instance                       = m_Instance;
    create_info.vulkanApiVersion               = VK_API_VERSION_1_3;
    create_info.pTypeExternalMemoryHandleTypes = nullptr;

    VK_CALL(vmaCreateAllocator(&create_info, &m_Allocator));
  }

  void VulkanState::InitTracy() {
    m_TracyCmdBuf = GetCommandPool(VK_QUEUE_GRAPHICS_BIT)->AllocateCommandBuffers(1)[0];
    m_TracyContext = TracyVkContext(m_PhysicalDevice, m_Device->GetDevice(), m_Device->GetGraphicsQueue()->Get(), m_TracyCmdBuf->Get());
  }

  void VulkanState::ShutdownTracy() {
    m_TracyCmdBuf = nullptr;
    TracyVkDestroy(m_TracyContext);
    m_TracyContext = nullptr;
  }

}
