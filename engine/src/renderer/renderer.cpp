#include "renderer.h"

#include <glm/glm.hpp>
#include "imgui_renderer.h"

namespace mau {

  Renderer::Renderer(void* window_ptr) {
    Handle<CommandPool> cmd_pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();

    // create renderpass
    m_Renderpass = make_handle<Renderpass>();
    LoadStoreOp op;
    m_Renderpass->AddColorAttachment(swapchain->GetColorFormat(), VK_SAMPLE_COUNT_1_BIT, op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_Renderpass->SetDepthAttachment(swapchain->GetDepthFormat(), VK_SAMPLE_COUNT_1_BIT, op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    m_Renderpass->Build(VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    // create pipeline
    m_VertexShader = make_handle<VertexShader>(GetAssetFolderPath() + "shaders/basic_vertex.glsl");
    m_FragmentShader = make_handle<FragmentShader>(GetAssetFolderPath() + "shaders/basic_fragment.glsl");

    InputLayout input_layout;
    input_layout.AddBindingDesc(0u, sizeof(glm::vec2));
    input_layout.AddAttributeDesc(0u, 0u, VK_FORMAT_R32G32_SFLOAT, 0u);

    m_Pipeline = make_handle<Pipeline>(m_VertexShader, m_FragmentShader, m_Renderpass, input_layout);

    // create framebuffers and sync objects
    std::vector<Handle<ImageView>> swapchain_images = swapchain->GetImageViews();
    std::vector<Handle<ImageView>> swapchain_depth_images = swapchain->GetDepthImageViews();
    m_Extent = swapchain->GetExtent();
    for (size_t i = 0; i < swapchain_images.size(); i++) {
      Handle<Framebuffer> fbo = make_handle<Framebuffer>(swapchain_images[i], swapchain_depth_images[i], m_Renderpass, m_Extent.width, m_Extent.height);
      m_Framebuffers.push_back(fbo);

      m_ImageAvailable.push_back(make_handle<Semaphore>());
      m_RenderFinished.push_back(make_handle<Semaphore>());
      m_QueueSubmit.push_back(make_handle<Fence>(VK_FENCE_CREATE_SIGNALED_BIT));
    }

    // allocate command buffers
    m_CommandBuffers = cmd_pool->AllocateCommandBuffers(static_cast<TUint32>(swapchain_images.size()));

    // create vertex/index buffers
    std::vector<glm::vec2> data = {
      glm::vec2(-0.5, -0.5),
      glm::vec2(0.5, 0.5),
      glm::vec2(-0.5, 0.5),
      glm::vec2(0.5, -0.5),
    };
    m_QuadBuffer = make_handle<VertexBuffer>(data.size() * sizeof(data[0]), data.data());

    TUint32 indices[] = { 0, 1, 2, 0, 3, 1 };

    m_QuadIndices = make_handle<IndexBuffer>(sizeof(indices), indices);

    // init imgui
    ImguiRenderer::Create(m_Renderpass, window_ptr);
  }

  Renderer::~Renderer() {
    ImguiRenderer::Destroy();
  }

  void Renderer::StartFrame() {
    ImguiRenderer::Ref().StartFrame();
  }

  void Renderer::EndFrame() {
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice> device = VulkanState::Ref().GetDeviceHandle();

    Handle<Fence> queue_submit = m_QueueSubmit[m_CurrentFrame];
    vkWaitForFences(device->GetDevice(), 1, queue_submit->Ref(), VK_TRUE, UINT64_MAX);

    Handle<Semaphore> image_available = m_ImageAvailable[m_CurrentFrame];
    Handle<Semaphore> render_finished = m_RenderFinished[m_CurrentFrame];
    TUint32 image_index = swapchain->GetNextImageIndex(image_available);
    vkResetFences(device->GetDevice(), 1, queue_submit->Ref());

    RecordCommandBuffer(static_cast<TUint64>(image_index));

    VkSubmitInfo submit_info = {};
    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = image_available->Ref();
    submit_info.pWaitDstStageMask = &wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = m_CommandBuffers[static_cast<TUint64>(image_index)]->Ref();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = render_finished->Ref();
    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submit_info, queue_submit->Get());

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = render_finished->Ref();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchain->Ref();
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    vkQueuePresentKHR(VulkanState::Ref().GetPresentQueue(), &present_info);

    m_CurrentFrame = (m_CurrentFrame + 1) % m_Framebuffers.size();
  }

  void Renderer::RecordCommandBuffer(TUint64 idx) {
    Handle<CommandBuffer> cmd = m_CommandBuffers[idx];
    cmd->Reset();
    cmd->Begin();

    VkClearValue clear_value[2] = {};
    clear_value[0].color = { 0.0f, 1.0f, 0.0f, 1.0f };
    clear_value[1].depthStencil = { 1.0f, 0u };
    VkRenderPassBeginInfo renderpass_begin_info = {};
    renderpass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.pNext = nullptr;
    renderpass_begin_info.renderPass = m_Renderpass->Get();
    renderpass_begin_info.framebuffer = m_Framebuffers[idx]->GetFramebuffer();
    renderpass_begin_info.renderArea = { { 0, 0 }, m_Extent };
    renderpass_begin_info.clearValueCount = ARRAY_SIZE(clear_value);
    renderpass_begin_info.pClearValues = clear_value;
    vkCmdBeginRenderPass(cmd->Get(), &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0.0f, 0.0f, m_Extent.width, m_Extent.height, 0.0f, 1.0f };
    VkRect2D scissor = { { 0, 0 }, m_Extent };

    vkCmdBindPipeline(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Get());

    vkCmdSetViewport(cmd->Get(), 0u, 1u, &viewport);
    vkCmdSetScissor(cmd->Get(), 0u, 1u, &scissor);

    VkDeviceSize offsets[] = { 0ui64 };
    vkCmdBindVertexBuffers(cmd->Get(), 0u, 1, m_QuadBuffer->Ref(), offsets);
    vkCmdBindIndexBuffer(cmd->Get(), m_QuadIndices->Get(), 0ui64, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd->Get(), 6, 1, 0, 0, 0);

    ImguiRenderer::Ref().EndFrame(cmd);

    vkCmdEndRenderPass(cmd->Get());
    cmd->End();
  }

}
