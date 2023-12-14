#include "imgui-renderer.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_glfw.cpp>

namespace mau {

  ImguiRenderer::ImguiRenderer(void* window) {
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice> device = VulkanState::Ref().GetDeviceHandle();

    // create renderpass
    Handle<Image> swapchain_image = swapchain->GetImages()[0];

    m_Renderpass = make_handle<Renderpass>();
    LoadStoreOp op;
    op.LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    m_Renderpass->AddColorAttachment(swapchain_image->GetFormat(), swapchain_image->GetSamples(), op, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_Renderpass->Build(VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    // create framebuffers
    std::vector<Handle<ImageView>> swapchain_images = swapchain->GetImageViews();
    m_Extent = swapchain->GetExtent();
    for (size_t i = 0; i < swapchain_images.size(); i++) {
      std::vector<Handle<ImageView>> image = { swapchain_images[i] };
      Handle<Framebuffer> fbo = make_handle<Framebuffer>(image, m_Renderpass, m_Extent.width, m_Extent.height);
      m_Framebuffers.push_back(fbo);
    }

    // init imgui
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

    GLFWwindow* glfw_window = reinterpret_cast<GLFWwindow*>(window);
    ImGui_ImplGlfw_InitForVulkan(glfw_window, true);
    ImGui_ImplVulkan_Init(&init_info, m_Renderpass->Get());

    Handle<CommandPool> pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<CommandBuffer> command_buffer = pool->AllocateCommandBuffers(1)[0];
    command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer->Get());

    command_buffer->End();
    Handle<VulkanQueue> graphics_queue = device->GetGraphicsQueue();
    graphics_queue->Submit(command_buffer);

    vkDeviceWaitIdle(device->GetDevice());

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    // recreate framebuffers on window resize
    swapchain->RegisterSwapchainCreateCallbackFunc([this]() -> void {
      Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
      auto new_swapchain_images = swapchain->GetImageViews();
      auto new_swapchain_depth_images = swapchain->GetDepthImageViews();
      m_Extent = swapchain->GetExtent();
      m_Framebuffers.clear();

      for (size_t i = 0; i < new_swapchain_images.size(); i++) {
        std::vector<Handle<ImageView>> image = { new_swapchain_images[i] };
        Handle<Framebuffer> fbo = make_handle<Framebuffer>(image, m_Renderpass, m_Extent.width, m_Extent.height);
        m_Framebuffers.push_back(fbo);
      }
      });
  }

  ImguiRenderer::~ImguiRenderer() {
    m_Framebuffers.clear();
    m_Renderpass = nullptr;
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

  void ImguiRenderer::EndFrame(Handle<CommandBuffer> cmd, TUint64 idx) {
    m_Renderpass->Begin(cmd, m_Framebuffers[idx], { { 0, 0 }, m_Extent });

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->Get());
    vkCmdEndRenderPass(cmd->Get());
  }

}
