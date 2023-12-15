#include "imgui-context.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_glfw.cpp>
#include <engine/events/key-events.h>
#include <engine/events/mouse-events.h>

namespace mau {

  ImGuiContext::ImGuiContext(void* window, Handle<Renderpass> renderpass) {
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice> device = VulkanState::Ref().GetDeviceHandle();

    // init imgui
    VkDescriptorPoolSize imgui_pool_sizes[] = {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000u },
    };
    VkDescriptorPoolCreateInfo descriptor_create_info = {};
    descriptor_create_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_create_info.pNext                      = nullptr;
    descriptor_create_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_create_info.maxSets                    = 1000u;
    descriptor_create_info.poolSizeCount              = static_cast<uint32_t>(ARRAY_SIZE(imgui_pool_sizes));
    descriptor_create_info.pPoolSizes                 = imgui_pool_sizes;

    VK_CALL(vkCreateDescriptorPool(VulkanState::Ref().GetDevice(), &descriptor_create_info, nullptr, &m_DescriptorPool));

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = VulkanState::Ref().GetInstance();
    init_info.PhysicalDevice            = VulkanState::Ref().GetPhysicalDevice();
    init_info.Device                    = device->GetDevice();
    init_info.QueueFamily               = device->GetGraphicsQueueIndex();
    init_info.Queue                     = device->GetGraphicsQueue()->Get();
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = m_DescriptorPool;
    init_info.Subpass                   = 0;
    init_info.MinImageCount             = swapchain->GetSurfaceCapabilities().minImageCount;
    init_info.ImageCount                = static_cast<uint32_t>(swapchain->GetImageViews().size());
    init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(window);
    ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
    ImGui_ImplVulkan_Init(&init_info, renderpass->Get());

    Handle<CommandPool> pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<CommandBuffer> command_buffer = pool->AllocateCommandBuffers(1)[0];
    command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer->Get());

    command_buffer->End();
    Handle<VulkanQueue> graphics_queue = device->GetGraphicsQueue();
    graphics_queue->Submit(command_buffer);

    vkDeviceWaitIdle(device->GetDevice());

    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  ImGuiContext::~ImGuiContext() {
    vkDestroyDescriptorPool(VulkanState::Ref().GetDevice(), m_DescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void ImGuiContext::StartFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiDockspace();
  }

  void ImGuiContext::OnEvent(Event& event) {
    auto handle_event = [](Event &e) -> void {
      e.Handle();
    };

    if (m_BlockEvents) {
      EventDispatcher dispatcher(event);
      dispatcher.Dispatch<MousePressEvent>(handle_event);
      dispatcher.Dispatch<KeyPressEvent>(handle_event);
    }
  }

  void ImGuiContext::ImGuiDockspace() {
    // dockspace
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

    // make dockspace fullscreen
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Mau Dockspace", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("Mau Dockspace Id");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    ImGui::End();
  }

}
