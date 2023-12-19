#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <backends/imgui_impl_vulkan.h>
#include <engine/input/input.h>
#include <vulkan/vulkan_core.h>

#include "context/imgui-context.h"
#include "glm/geometric.hpp"
#include "graphics/vulkan-bindless.h"
#include "graphics/vulkan-features.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/rendergraph/passes/lambertian-pass.h"
#include "renderer/rendergraph/passes/imgui-pass.h"
#include "scene/internal-components.h"
#include "context/imgui-context.h"
#include "optix/denoiser.h"

namespace mau {

  glm::mat4 getModelMatrix(const TransformComponent &transform) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.Position);
    model = glm::rotate(model, transform.Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, transform.Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    return model;
  }

  Renderer::Renderer(void *window_ptr) {
    Handle<CommandPool>     cmd_pool = VulkanState::Ref().GetCommandPool(VK_QUEUE_GRAPHICS_BIT);
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    m_Extent = swapchain->GetExtent();

    Handle<Image> swapchain_image = swapchain->GetImages()[0];
    Handle<Image> depth_image = swapchain->GetDepthImages()[0];

    // init camera uniform buffer
    const glm::vec2 window_size = glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight));
    CameraBuffer    buff = {
           .view_proj = m_Camera.GetMVP(window_size),
           .view_inverse = glm::inverse(m_Camera.GetView()),
           .proj_inverse = glm::inverse(m_Camera.GetProj(window_size)),
    };
    m_CameraBuffer = make_handle<StructuredUniformBuffer<CameraBuffer>>(std::move(buff));
    m_CameraBufferHandle = VulkanBindless::Ref().AddBuffer(m_CameraBuffer);

    // init push constant
    m_Camera.Position.z = -0.05f;

    VertexShaderData push_constant;
    push_constant.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    push_constant.mvp = m_Camera.GetMVP(glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight)));
    push_constant.camera_buffer_index = m_CameraBufferHandle;
    push_constant.current_frame = 0u;
    push_constant.dir_light_color = glm::vec4(1.0f, 1.0f, 1.0f, 24.0f);
    push_constant.dir_light_direction = glm::vec4(-0.138, 0.924, -0.356, 0.0f);
    m_PushConstant = make_handle<PushConstant<VertexShaderData>>(push_constant);

    // create rendergraph
    CreateViewportBuffers(m_ImGuiViewportWidth, m_ImGuiViewportHeight);
    std::vector<Sink> sinks = {sink_color, sink_depth};

    m_Rendergraph = make_handle<RenderGraph>();
    Handle<LambertianPass> pass = make_handle<LambertianPass>();
    Handle<ImGuiPass>      imgui_pass = make_handle<ImGuiPass>();
    m_Rendergraph->AddPass(pass);
    m_Rendergraph->AddPass(imgui_pass);
    m_Rendergraph->Build(sinks);

    // init imgui
    ImGuiContext::Create(window_ptr, imgui_pass->GetRenderpass());
    CreateImguiTextures();

    // create pipeline
    m_VertexShader = make_handle<VertexShader>(GetAssetFolderPath() + "shaders/basic_vertex.glsl");
    m_FragmentShader = make_handle<FragmentShader>(GetAssetFolderPath() + "shaders/basic_fragment.glsl");

    InputLayout input_layout;
    input_layout.AddBindingDesc(0u, (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2)));
    input_layout.AddAttributeDesc(0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, 0u);
    input_layout.AddAttributeDesc(1u, 0u, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3));
    input_layout.AddAttributeDesc(2u, 0u, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec3));

    m_Pipeline = make_handle<Pipeline>(m_VertexShader, m_FragmentShader, pass->GetRenderpass(), input_layout, m_PushConstant, VulkanBindless::Ref().GetDescriptorLayout(), VK_SAMPLE_COUNT_4_BIT);

    if (VulkanFeatures::IsRtEnabled()) {
      m_RTCHit = make_handle<RTClosestHitShader>(GetAssetFolderPath() + "shaders/rt/basic.rchit");
      m_RTGen = make_handle<RTRayGenShader>(GetAssetFolderPath() + "shaders/rt/basic.rgen");
      m_RTMiss = make_handle<RTMissShader>(GetAssetFolderPath() + "shaders/rt/basic.rmiss");

      RTPipelineCreateInfo rt_pipeline_info = {
          .ClosestHit = m_RTCHit,
          .RayGen = m_RTGen,
          .Miss = m_RTMiss,
          .PushConstant = m_PushConstant,
          .DescriptorLayouts = VulkanBindless::Ref().GetDescriptorLayout(),
      };

      m_RTPipeline = make_handle<RTPipeline>(rt_pipeline_info);
    }

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

    // recreate framebuffers on window resize
    swapchain->RegisterSwapchainCreateCallbackFunc([this]() -> void {
      std::vector<Sink> sinks = {sink_color, sink_depth, sink_accum};
      m_Rendergraph->Build(sinks);
      Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
      m_Extent = swapchain->GetExtent();
    });

    // initial flag buffer
    for (size_t i = 0; i < swapchain_images.size(); i++) {
      m_ClearAccumFlag.push_back(true);
    }
  }

  Renderer::~Renderer() { ImGuiContext::Destroy(); }

  void Renderer::StartFrame() {
    MAU_PROFILE_SCOPE("Renderer::StartFrame");
    UpdateCamera();
    ImGuiContext::Ref().StartFrame();
  }

  void Renderer::EndFrame() {
    MAU_PROFILE_SCOPE("Renderer::EndFrame");
    Handle<VulkanSwapchain> swapchain = VulkanState::Ref().GetSwapchainHandle();
    Handle<VulkanDevice>    device = VulkanState::Ref().GetDeviceHandle();

    Handle<Fence> queue_submit = m_QueueSubmit[m_CurrentFrame];
    queue_submit->Wait();

    Handle<Semaphore> image_available = m_ImageAvailable[m_CurrentFrame];
    Handle<Semaphore> render_finished = m_RenderFinished[m_CurrentFrame];
    TUint32           image_index = swapchain->GetNextImageIndex(image_available);
    queue_submit->Reset();

    // recreate render target on viewport resize
    if (m_CurrentViewportWidth != m_ImGuiViewportWidth || m_CurrentViewportHeight != m_ImGuiViewportHeight) {
      m_ImGuiViewportWidth = m_CurrentViewportWidth;
      m_ImGuiViewportHeight = m_CurrentViewportHeight;

      vkDeviceWaitIdle(device->GetDevice());

      CreateViewportBuffers(m_ImGuiViewportWidth, m_ImGuiViewportHeight);
      CreateImguiTextures();
      std::vector<Sink> sinks = {sink_color, sink_depth, sink_accum};
      m_Rendergraph->Build(sinks);

      VertexShaderData data = m_PushConstant->GetData();
      data.mvp = m_Camera.GetMVP(glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight)));
      m_PushConstant->Update(data);
    }

    ImGuiTest(image_index);

    RecordCommandBuffer(static_cast<TUint64>(image_index));

    Handle<VulkanQueue>  graphics_queue = device->GetGraphicsQueue();
    Handle<PresentQueue> present_queue = device->GetPresentQueue();

    graphics_queue->Submit(m_CommandBuffers[static_cast<TUint64>(image_index)], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, image_available, render_finished, queue_submit);
    present_queue->Present(image_index, swapchain, render_finished);

    m_CurrentFrame = (m_CurrentFrame + 1) % swapchain->GetImages().size();
  }

  void Renderer::Render(Handle<CommandBuffer> cmd, TUint32 frame_index) {
    const glm::vec2 window_size = glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight));
    CameraBuffer    buff = {
           .view_proj = m_Camera.GetMVP(window_size),
           .view_inverse = glm::inverse(m_Camera.GetView()),
           .proj_inverse = glm::inverse(m_Camera.GetProj(window_size)),
    };
    m_CameraBuffer->Update(std::move(buff));

    m_PushConstant->Bind(cmd, m_Pipeline);
    vkCmdBindPipeline(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Get());

    const std::vector<VkDescriptorSet> &sets = VulkanBindless::Ref().GetDescriptorSet();
    vkCmdBindDescriptorSets(cmd->Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetLayout(), 0u, static_cast<TUint32>(sets.size()), sets.data(), 0u, nullptr);

    if (m_DrawScene) {
      m_DrawScene->Each([this, &cmd](Entity entity) -> void {
        VkDeviceSize        offsets[] = {0u};
        TransformComponent &transform = entity.Get<TransformComponent>();
        MeshComponent      &mesh = entity.Get<MeshComponent>();

        glm::mat4 mvp = m_Camera.GetMVP(glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight))) * getModelMatrix(transform);
        for (const auto &submesh : mesh.MeshObject->GetSubMeshes()) {
          m_PushConstant->Update({
              .color = m_PushConstant->GetData().color,
              .mvp = mvp,
              .material_index = submesh.GetMaterial() ? submesh.GetMaterial()->GetMaterialHandle() : UINT32_MAX,
              .storage_image_index = UINT32_MAX,
              .camera_buffer_index = m_CameraBufferHandle,
              .accum_image_index = UINT32_MAX,
          });
          m_PushConstant->Bind(cmd, m_Pipeline);

          vkCmdBindVertexBuffers(cmd->Get(), 0u, 1u, submesh.GetVertexBuffer()->Ref(), offsets);
          vkCmdBindIndexBuffer(cmd->Get(), submesh.GetIndexBuffer()->Get(), 0u, VK_INDEX_TYPE_UINT32);
          vkCmdDrawIndexed(cmd->Get(), submesh.GetIndexCount(), 1, 0, 0, 0);
        }
      });
    }
    m_DrawScene = nullptr;
  }

  void Renderer::RenderRT(Handle<CommandBuffer> cmd, TUint32 frame_index) {
    const glm::vec2 window_size = glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight));
    CameraBuffer    buff = {
           .view_proj = m_Camera.GetMVP(window_size),
           .view_inverse = glm::inverse(m_Camera.GetView()),
           .proj_inverse = glm::inverse(m_Camera.GetProj(window_size)),
    };
    m_CameraBuffer->Update(std::move(buff));

    vkCmdBindPipeline(cmd->Get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTPipeline->Get());

    const Vector<VkDescriptorSet> &sets = VulkanBindless::Ref().GetDescriptorSet();
    vkCmdBindDescriptorSets(cmd->Get(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTPipeline->GetLayout(), 0u, static_cast<TUint32>(sets.size()), sets.data(), 0u, nullptr);

    glm::mat4     mvp = m_Camera.GetMVP(glm::vec2(static_cast<float>(m_ImGuiViewportWidth), static_cast<float>(m_ImGuiViewportHeight)));
    const TUint32 current_frame = m_ClearAccumFlag[frame_index] ? 0 : m_PushConstant->GetData().current_frame + 1;
    m_ClearAccumFlag[frame_index] = false;
    m_PushConstant->Update({
        .color = m_PushConstant->GetData().color,
        .mvp = mvp,
        .material_index = 0u,
        .storage_image_index = sink_color_handles[frame_index],
        .camera_buffer_index = m_CameraBufferHandle,
        .current_frame = current_frame,
        .accum_image_index = sink_accum_handles[frame_index],
        .albedo_image_index = sink_albedo_handles[frame_index],
        .normal_image_index = sink_normal_handles[frame_index],
        .dir_light_color = m_PushConstant->GetData().dir_light_color,
        .dir_light_direction = m_PushConstant->GetData().dir_light_direction,
    });
    m_PushConstant->Bind(cmd, m_RTPipeline);

    if (m_DrawScene) {
      bool transform_updated = false;

      m_DrawScene->Each([this, &cmd, &transform_updated](Entity entity) -> void {
        TransformComponent &transform = entity.Get<TransformComponent>();
        MeshComponent      &mesh = entity.Get<MeshComponent>();

        if (transform.Updated)
          transform_updated = true;

        glm::mat4 model = getModelMatrix(transform);
        mesh.MeshObject->GetAccel()->UpdateTransform(model, cmd);
      });

      RTSBTRegion region = m_RTPipeline->GetSBTRegion();
      vkCmdTraceRaysKHR(cmd->Get(), &region.RayGen, &region.RayMiss, &region.RayClosestHit, &region.RayCall, m_ImGuiViewportWidth, m_ImGuiViewportHeight, 1);

      Handle<ImageResource> current_image = sink_color.GetResource(frame_index);
      Handle<ImageResource> current_albedo = sink_albedo.GetResource(frame_index);
      Handle<ImageResource> current_normal = sink_normal.GetResource(frame_index);

      if (EnableDenoiser) {
        Denoiser::Ref().ImageToBuffers(cmd, current_image->GetImage(), current_albedo->GetImage(), current_normal->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        Denoiser::Ref().Denoise(cmd);
        Denoiser::Ref().BufferToImage(cmd, current_image->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      }

      // TransitionImageLayout(cmd, current_image->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      if (transform_updated) {
        for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
          m_ClearAccumFlag[i] = true;
      }
    }

    m_DrawScene = nullptr;
  }

  void Renderer::UpdateCamera() {
    bool            updated = false;
    const float     sensitivity = 0.1f;
    const float     mouse_sensitivity = 0.003f;
    const glm::vec3 right = glm::cross(m_Camera.Up, m_Camera.Direction);

    if (Input::IsKeyDown(MAU_KEY_W)) {
      m_Camera.Position += m_Camera.Direction * sensitivity;
      updated = true;
    }
    if (Input::IsKeyDown(MAU_KEY_S)) {
      m_Camera.Position -= m_Camera.Direction * sensitivity;
      updated = true;
    }
    if (Input::IsKeyDown(MAU_KEY_A)) {
      m_Camera.Position += right * sensitivity;
      updated = true;
    }
    if (Input::IsKeyDown(MAU_KEY_D)) {
      m_Camera.Position -= right * sensitivity;
      updated = true;
    }

    if (Input::IsMouseDown(MAU_MOUSE_BUTTON_LEFT)) {
      const glm::vec2 mouse_offset = Input::GetMouseOffset();
      const glm::vec3 offset = m_Camera.Up * -mouse_offset.y * mouse_sensitivity + right * -mouse_offset.x * mouse_sensitivity;
      m_Camera.Direction = glm::normalize(m_Camera.Direction + offset);
      updated = true;
    }
    const float scroll = Input::GetMouseScroll();
    if (scroll != 0.0f) {
      m_Camera.Position += m_Camera.Direction * scroll;
      updated = true;
    }

    if (updated) {
      for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
        m_ClearAccumFlag[i] = true;
    }
  }

  void Renderer::RecordCommandBuffer(TUint64 idx) {
    MAU_PROFILE_SCOPE("Renderer::RecordCommandBuffer");

    Handle<CommandBuffer> cmd = m_CommandBuffers[idx];
    cmd->Reset();
    cmd->Begin();

    m_Rendergraph->Execute(cmd, idx);

    MAU_GPU_COLLECT(cmd->Get());
    cmd->End();
  }

  void Renderer::ImGuiTest(TUint32 idx) {
    VertexShaderData data = m_PushConstant->GetData();

    if (ImGui::Begin("Test Window")) {
      if (ImGui::DragFloat3("Position", &m_Camera.Position[0])) {
        for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
          m_ClearAccumFlag[i] = true;
      }

      if (ImGui::ColorEdit3("Global Light Color", &data.dir_light_color[0])) {
        m_PushConstant->Update(data);
        for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
          m_ClearAccumFlag[i] = true;
      }

      if (ImGui::DragFloat("Global Light Intensity", &data.dir_light_color.w)) {
        m_PushConstant->Update(data);
        for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
          m_ClearAccumFlag[i] = true;
      }

      if (ImGui::SliderFloat3("Global Light Direction", &data.dir_light_direction[0], -1.0f, 1.0f)) {
        glm::vec3 new_dir = data.dir_light_direction;
        new_dir = glm::normalize(new_dir);
        data.dir_light_direction.x = new_dir.x;
        data.dir_light_direction.y = new_dir.y;
        data.dir_light_direction.z = new_dir.z;
        m_PushConstant->Update(data);
        for (size_t i = 0; i < m_ClearAccumFlag.size(); i++)
          m_ClearAccumFlag[i] = true;
      }
    }
    ImGui::End();

    if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse)) {
      const bool viewport_hovered = ImGui::IsWindowHovered();
      ImGuiContext::Ref().BlockEvents(!viewport_hovered);

      const ImVec2 viewport_size = ImVec2(m_ImGuiViewportWidth, m_ImGuiViewportHeight);
      ImVec2       avail_size = ImGui::GetContentRegionAvail();

      m_CurrentViewportWidth = static_cast<TUint32>(avail_size.x);
      m_CurrentViewportHeight = static_cast<TUint32>(avail_size.y);

      ImGui::Image(imgui_texture_ids[idx], viewport_size);
    }
    ImGui::End();
  }

  void Renderer::CreateViewportBuffers(TUint32 width, TUint32 height) {
    sink_color_handles.clear();
    sink_accum_handles.clear();
    sink_albedo_handles.clear();
    sink_normal_handles.clear();

    const VkFormat color_format = VulkanState::Ref().GetSwapchainColorFormat();
    const VkFormat depth_format = VulkanState::Ref().GetSwapchainDepthFormat();
    const TUint64  image_count = VulkanState::Ref().GetSwapchainImageViews().size();

    std::vector<Handle<Resource>> color_images = {};
    std::vector<Handle<Resource>> depth_images = {};
    std::vector<Handle<Resource>> accum_images = {};
    std::vector<Handle<Resource>> albedo_images = {};
    std::vector<Handle<Resource>> normal_images = {};

    for (TUint64 i = 0; i < image_count; i++) {
      Handle<Image>     color = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
      Handle<ImageView> color_view = make_handle<ImageView>(color, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

      Handle<Image>     depth = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
      Handle<ImageView> depth_view = make_handle<ImageView>(depth, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

      Handle<Image>     accum = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
      Handle<ImageView> accum_view = make_handle<ImageView>(accum, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

      Handle<Image>     alb = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
      Handle<ImageView> alb_view = make_handle<ImageView>(alb, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

      Handle<Image>     norm = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
      Handle<ImageView> norm_view = make_handle<ImageView>(norm, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

      color_images.push_back(make_handle<ImageResource>(color, color_view));
      depth_images.push_back(make_handle<ImageResource>(depth, depth_view));
      accum_images.push_back(make_handle<ImageResource>(accum, accum_view));
      albedo_images.push_back(make_handle<ImageResource>(alb, alb_view));
      normal_images.push_back(make_handle<ImageResource>(norm, norm_view));

      if (VulkanFeatures::IsRtEnabled()) {
        sink_color_handles.push_back(VulkanBindless::Ref().AddStorageImage(color_view));
        sink_accum_handles.push_back(VulkanBindless::Ref().AddStorageImage(accum_view));
        sink_albedo_handles.push_back(VulkanBindless::Ref().AddStorageImage(alb_view));
        sink_normal_handles.push_back(VulkanBindless::Ref().AddStorageImage(norm_view));
      }
    }

    sink_color.AssignResources(color_images);
    sink_depth.AssignResources(depth_images);
    sink_accum.AssignResources(accum_images);
    sink_albedo.AssignResources(albedo_images);
    sink_normal.AssignResources(normal_images);

    Denoiser::Ref().AllocateBuffers(width, height);
  }

  void Renderer::CreateImguiTextures() {
    for (const auto &texture : imgui_texture_ids) {
      ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
    }

    const TUint64 image_count = VulkanState::Ref().GetSwapchainImageViews().size();
    imgui_texture_ids.clear();

    for (TUint64 i = 0; i < image_count; i++) {
      Handle<ImageResource> resource = sink_color.GetResource(i);

      ImTextureID texture_id = ImGui_ImplVulkan_AddTexture(sampler.Get(), resource->GetImageView()->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imgui_texture_ids.push_back(reinterpret_cast<void *>(texture_id));
    }
  }

} // namespace mau
