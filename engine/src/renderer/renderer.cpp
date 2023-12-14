#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui-renderer.h"
#include "graphics/vulkan-bindless.h"
#include "renderer/rendergraph/passes/lambertian-pass.h"

namespace mau {

  void imgui_test(Handle<PushConstant<VertexShaderData>> push_constant, Handle<StructuredUniformBuffer<ShaderData>> uniform_buffer, Camera* camera, VkExtent2D extent) {
    VertexShaderData data = push_constant->GetData();
    static glm::vec3 model_pos(0.0f);
    static glm::vec3 model_rot(0.0f);

    if (ImGui::Begin("Test Window")) {
      if (ImGui::ColorEdit4("Quad Color", glm::value_ptr(data.color))) {
        push_constant->Update(data);
      }

      if (ImGui::DragFloat3("Position", &camera->Position[0])) {
        data.mvp = camera->GetMVP(glm::vec2(static_cast<float>(extent.width), static_cast<float>(extent.height)));
        push_constant->Update(data);
      }

      if (ImGui::DragFloat3("Model Position", &model_pos[0])) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), model_pos);
        model = glm::rotate(model, model_rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, model_rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, model_rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

        uniform_buffer->Update({
          .model = model,
        });
      }

      if (ImGui::DragFloat3("Model Rotation", &model_rot[0])) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), model_pos);
        model = glm::rotate(model, model_rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, model_rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, model_rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

        uniform_buffer->Update({
          .model = model,
        });
      }
    }
    ImGui::End();
  }

  Renderer::Renderer(void* window_ptr) {
    Handle<CommandPool> cmd_pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    m_Extent = swapchain->GetExtent();

    Handle<Image> swapchain_image = swapchain->GetImages()[0];
    Handle<Image> depth_image = swapchain->GetDepthImages()[0];

    // init uniform buffer
    ShaderData model_data;
    model_data.model = glm::mat4(1.0f);
    m_UniformBuffer = make_handle<StructuredUniformBuffer<ShaderData>>();
    m_UniformBuffer->Update(std::move(model_data));
    const BufferHandle buffer_handle = VulkanBindless::Ref().AddBuffer(m_UniformBuffer);

    // init push constant
    VertexShaderData push_constant;
    push_constant.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    push_constant.mvp = m_Camera.GetMVP(glm::vec2(static_cast<float>(m_Extent.width), static_cast<float>(m_Extent.height)));
    push_constant.ubo_index = buffer_handle;
    m_PushConstant = make_handle<PushConstant<VertexShaderData>>(push_constant);

    // create rendergraph
    m_Rendergraph = make_handle<RenderGraph>();
    Handle<LambertianPass> pass = make_handle<LambertianPass>();
    m_Rendergraph->AddPass(pass);
    m_Rendergraph->Build();

    // create pipeline
    m_VertexShader = make_handle<VertexShader>(GetAssetFolderPath() + "shaders/basic_vertex.glsl");
    m_FragmentShader = make_handle<FragmentShader>(GetAssetFolderPath() + "shaders/basic_fragment.glsl");

    InputLayout input_layout;
    input_layout.AddBindingDesc(0u, (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)));
    input_layout.AddAttributeDesc(0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, 0u);
    input_layout.AddAttributeDesc(1u, 0u, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3));
    input_layout.AddAttributeDesc(2u, 0u, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec3));

    m_Pipeline = make_handle<Pipeline>(m_VertexShader, m_FragmentShader, pass->GetRenderpass(), input_layout, m_PushConstant, VulkanBindless::Ref().GetDescriptorLayout());

    // create framebuffers and sync objects
    std::vector<Handle<ImageView>> swapchain_images = swapchain->GetImageViews();
    std::vector<Handle<ImageView>> swapchain_depth_images = swapchain->GetDepthImageViews();
    for (size_t i = 0; i < swapchain_images.size(); i++) {
      m_ImageAvailable.push_back(make_handle<Semaphore>());
      m_RenderFinished.push_back(make_handle<Semaphore>());
      m_QueueSubmit.push_back(make_handle<Fence>(VK_FENCE_CREATE_SIGNALED_BIT));
    }

    // allocate command buffers
    m_CommandBuffers = cmd_pool->AllocateCommandBuffers(static_cast<TUint32>(swapchain_images.size()));

    // create vertex/index buffers
    String model_path = GetAssetFolderPath() + "assets/models/backpack/backpack.obj";
    m_Mesh = make_handle<Mesh>(model_path);
    m_Texture = make_handle<Texture>(GetAssetFolderPath() + "assets/models/backpack/diffuse.jpg");

    m_TextureHandle = VulkanBindless::Ref().AddTexture(m_Texture);

    // init imgui
    ImguiRenderer::Create(window_ptr);

    // recreate framebuffers on window resize
    swapchain->RegisterSwapchainCreateCallbackFunc([this]() -> void {
      m_Rendergraph->Build();
      Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
      m_Extent = swapchain->GetExtent();
    });
  }

  Renderer::~Renderer() {
    ImguiRenderer::Destroy();
  }

  void Renderer::StartFrame() {
    MAU_PROFILE_SCOPE("Renderer::StartFrame");
    ImguiRenderer::Ref().StartFrame();

    imgui_test(m_PushConstant, m_UniformBuffer, &m_Camera, m_Extent);
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

    m_CurrentFrame = (m_CurrentFrame + 1) % swapchain->GetImages().size();
  }

  void Renderer::Render(Handle<CommandBuffer> cmd, TUint32 frame_index) {
    m_PushConstant->Bind(cmd, m_Pipeline);
    vkCmdBindPipeline(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Get());

    const std::vector<VkDescriptorSet>& sets = VulkanBindless::Ref().GetDescriptorSet();
    vkCmdBindDescriptorSets(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetLayout(), 0u, static_cast<TUint32>(sets.size()), sets.data(), 0u, nullptr);

    VkDeviceSize offsets[] = { 0ui64 };
    vkCmdBindVertexBuffers(cmd->Get(), 0u, 1u, m_Mesh->GetVertexBuffer()->Ref(), offsets);
    vkCmdBindIndexBuffer(cmd->Get(), m_Mesh->GetIndexBuffer()->Get(), 0ui64, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd->Get(), m_Mesh->GetIndexCount(), 1, 0, 0, 0);
  }

  void Renderer::RecordCommandBuffer(TUint64 idx) {
    MAU_PROFILE_SCOPE("Renderer::RecordCommandBuffer");

    Handle<CommandBuffer> cmd = m_CommandBuffers[idx];
    cmd->Reset();
    cmd->Begin();

    {
      MAU_GPU_ZONE(cmd->Get(), "Renderer::RenderQuad");
      m_Rendergraph->Execute(cmd, idx);
    }

    {
      MAU_GPU_ZONE(cmd->Get(), "Renderer::ImGui");
      ImguiRenderer::Ref().EndFrame(cmd, idx);
    }

    MAU_GPU_COLLECT(cmd->Get());
    cmd->End();
  }

}
