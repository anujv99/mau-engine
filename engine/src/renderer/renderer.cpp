#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui_renderer.h"

namespace mau {

  void imgui_test(Handle<PushConstant<VertexShaderData>> push_constant, Handle<StructuredUniformBuffer<ShaderCameraData>> uniform_buffer) {
    VertexShaderData data = push_constant->GetData();
    static float rotation = 0;

    if (ImGui::Begin("Test Window")) {
      if (ImGui::ColorEdit4("Quad Color", glm::value_ptr(data.color))) {
        push_constant->Update(data);
      }

      if (ImGui::SliderFloat("Rotation", &rotation, 0, 360)) {
        uniform_buffer->Update({
          .mvp = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f)),
        });
      }
    }
    ImGui::End();
  }

  Renderer::Renderer(void* window_ptr) {
    Handle<CommandPool> cmd_pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();

    Handle<Image> swapchain_image = swapchain->GetImages()[0];
    Handle<Image> depth_image = swapchain->GetDepthImages()[0];

    // init push constant
    VertexShaderData push_constant;
    push_constant.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    m_PushConstant = make_handle<PushConstant<VertexShaderData>>(push_constant);

    // create renderpass
    m_Renderpass = make_handle<Renderpass>();
    LoadStoreOp op;
    m_Renderpass->AddColorAttachment(swapchain_image->GetFormat(), swapchain_image->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_Renderpass->SetDepthAttachment(depth_image->GetFormat(), depth_image->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    m_Renderpass->Build(VK_PIPELINE_BIND_POINT_GRAPHICS, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    // create pipeline
    m_VertexShader = make_handle<VertexShader>(GetAssetFolderPath() + "shaders/basic_vertex.glsl");
    m_FragmentShader = make_handle<FragmentShader>(GetAssetFolderPath() + "shaders/basic_fragment.glsl");

    InputLayout input_layout;
    input_layout.AddBindingDesc(0u, sizeof(glm::vec2));
    input_layout.AddAttributeDesc(0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, 0u);

    DescriptorLayout descriptor_layout;
    descriptor_layout.AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    m_Pipeline = make_handle<Pipeline>(m_VertexShader, m_FragmentShader, m_Renderpass, input_layout, m_PushConstant, descriptor_layout);

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
    String sponza_path = GetAssetFolderPath() + "/assets/models/sponza/sponza.obj";
    m_Mesh = make_handle<Mesh>(sponza_path);

    // init imgui
    ImguiRenderer::Create(window_ptr);

    // recreate framebuffers on window resize
    swapchain->RegisterSwapchainCreateCallbackFunc([this]() -> void {
      Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
      auto new_swapchain_images = swapchain->GetImageViews();
      auto new_swapchain_depth_images = swapchain->GetDepthImageViews();
      m_Extent = swapchain->GetExtent();
      m_Framebuffers.clear();

      for (size_t i = 0; i < new_swapchain_images.size(); i++) {
        Handle<Framebuffer> fbo = make_handle<Framebuffer>(new_swapchain_images[i], new_swapchain_depth_images[i], m_Renderpass, m_Extent.width, m_Extent.height);
        m_Framebuffers.push_back(fbo);
      }
    });

    // create uniform buffers
    m_UniformBuffers = new StructuredUniformBuffer<ShaderCameraData>({
      .mvp = glm::mat4(1.0f),
    });
  }

  Renderer::~Renderer() {
    ImguiRenderer::Destroy();
  }

  void Renderer::StartFrame() {
    MAU_PROFILE_SCOPE("Renderer::StartFrame");
    ImguiRenderer::Ref().StartFrame();

    imgui_test(m_PushConstant, m_UniformBuffers);
  }

  void Renderer::EndFrame() {
    MAU_PROFILE_SCOPE("Renderer::EndFrame");
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice> device = VulkanState::Ref().GetDeviceHandle();

    Handle<Fence> queue_submit = m_QueueSubmit[m_CurrentFrame];
    queue_submit->Wait();

    Handle<Semaphore> image_available = m_ImageAvailable[m_CurrentFrame];
    Handle<Semaphore> render_finished = m_RenderFinished[m_CurrentFrame];
    TUint32 image_index = swapchain->GetNextImageIndex(image_available);
    queue_submit->Reset();

    RecordCommandBuffer(static_cast<TUint64>(image_index));

    Handle<VulkanQueue> graphics_queue = device->GetGraphicsQueue();
    Handle<PresentQueue> present_queue = device->GetPresentQueue();

    graphics_queue->Submit(m_CommandBuffers[static_cast<TUint64>(image_index)], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, image_available, render_finished, queue_submit);
    present_queue->Present(image_index, swapchain, render_finished);

    m_CurrentFrame = (m_CurrentFrame + 1) % m_Framebuffers.size();
  }

  void Renderer::RecordCommandBuffer(TUint64 idx) {
    MAU_PROFILE_SCOPE("Renderer::RecordCommandBuffer");

    Handle<CommandBuffer> cmd = m_CommandBuffers[idx];
    cmd->Reset();
    cmd->Begin();

    {
      MAU_GPU_ZONE(cmd->Get(), "Renderer::RenderQuad");
      m_Renderpass->Begin(cmd, m_Framebuffers[idx], { { 0, 0 }, m_Extent });

      VkViewport viewport = { 0.0f, 0.0f, static_cast<float>(m_Extent.width), static_cast<float>(m_Extent.height), 0.0f, 1.0f };
      VkRect2D scissor = { { 0, 0 }, m_Extent };

      m_PushConstant->Bind(cmd, m_Pipeline);
      vkCmdBindPipeline(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Get());
      vkCmdSetViewport(cmd->Get(), 0u, 1u, &viewport);
      vkCmdSetScissor(cmd->Get(), 0u, 1u, &scissor);

      // temp
      VkDescriptorBufferInfo uniform_descriptor_info = m_UniformBuffers->GetDescriptorInfo();
      VkWriteDescriptorSet write_descriptor_sets = {};
      write_descriptor_sets.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write_descriptor_sets.pNext                = nullptr;
      write_descriptor_sets.dstSet               = 0;
      write_descriptor_sets.dstBinding           = 0;
      write_descriptor_sets.descriptorCount      = 1;
      write_descriptor_sets.descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write_descriptor_sets.pBufferInfo          = &uniform_descriptor_info;

      vkCmdPushDescriptorSetKHR(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetLayout(), 0, 1, &write_descriptor_sets);

      VkDeviceSize offsets[] = { 0ui64 };
      vkCmdBindVertexBuffers(cmd->Get(), 0u, 1, m_Mesh->GetVertexBuffer()->Ref(), offsets);
      vkCmdBindIndexBuffer(cmd->Get(), m_Mesh->GetIndexBuffer()->Get(), 0ui64, VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(cmd->Get(), m_Mesh->GetIndexCount(), 1, 0, 0, 0);
      vkCmdEndRenderPass(cmd->Get());
    }

    {
      MAU_GPU_ZONE(cmd->Get(), "Renderer::ImGui");

      ImguiRenderer::Ref().EndFrame(cmd, idx);
    }

    MAU_GPU_COLLECT(cmd->Get());
    cmd->End();
  }

}