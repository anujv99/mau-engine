#include "imgui_renderer.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_glfw.cpp>

namespace mau {

  ImguiRenderer::ImguiRenderer(Handle<Renderpass> renderpass, void* window) {
    VkDescriptorPoolSize imgui_pool_sizes[] = {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };
    VkDescriptorPoolCreateInfo descriptor_create_info = {};
    descriptor_create_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_create_info.pNext                      = nullptr;
    descriptor_create_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_create_info.maxSets                    = 1u;
    descriptor_create_info.poolSizeCount              = static_cast<uint32_t>(ARRAY_SIZE(imgui_pool_sizes));
    descriptor_create_info.pPoolSizes                 = imgui_pool_sizes;

    VK_CALL(vkCreateDescriptorPool(VulkanState::Ref().GetDevice(), &descriptor_create_info, nullptr, &m_DescriptorPool));

    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice> device = VulkanState::Ref().GetDeviceHandle();

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = VulkanState::Ref().GetInstance();
    init_info.PhysicalDevice            = VulkanState::Ref().GetPhysicalDevice();
    init_info.Device                    = device->GetDevice();
    init_info.QueueFamily               = device->GetGraphicsQueueIndex();
    init_info.Queue                     = device->GetGraphicsQueue();
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = m_DescriptorPool;
    init_info.Subpass                   = 0;
    init_info.MinImageCount             = swapchain->GetSurfaceCapabilities().minImageCount;
    init_info.ImageCount                = static_cast<uint32_t>(swapchain->GetImageViews().size());
    init_info.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(window);
    ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
    ImGui_ImplVulkan_Init(&init_info, renderpass->Get());

    Handle<CommandPool> pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<CommandBuffer> command_buffer = pool->AllocateCommandBuffers(1)[0];
    command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer->Get());

    VkSubmitInfo submit_info         = {};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                = nullptr;
    submit_info.waitSemaphoreCount   = 0u;
    submit_info.pWaitSemaphores      = nullptr;
    submit_info.pWaitDstStageMask    = nullptr;
    submit_info.commandBufferCount   = 1u;
    submit_info.pCommandBuffers      = command_buffer->Ref();
    submit_info.signalSemaphoreCount = 0u;
    submit_info.pSignalSemaphores    = nullptr;

    command_buffer->End();
    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);

    vkDeviceWaitIdle(device->GetDevice());

    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }

  ImguiRenderer::~ImguiRenderer() {
    vkDestroyDescriptorPool(VulkanState::Ref().GetDevice(), m_DescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void ImguiRenderer::StartFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  void ImguiRenderer::EndFrame(Handle<CommandBuffer> buffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer->Get());
  }

}
