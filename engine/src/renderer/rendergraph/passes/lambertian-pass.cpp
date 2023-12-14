#include "lambertian-pass.h"

#include "renderer/renderer.h"
#include "graphics/vulkan-state.h"

namespace mau {

  LambertianPass::LambertianPass(): Pass("lambertian-pass") {
    RegisterSource("imgui-viewport-color");
    RegisterSource("imgui-viewport-depth");
  }

  LambertianPass::~LambertianPass() { }

  bool LambertianPass::PostBuild(TUint32 swapchain_image_count) {
    m_Framebuffers.clear();

    const Source& source = m_Sources.at("imgui-viewport-color");
    Handle<ImageResource> source_image = source.GetResource(0u);

    const Source& depth_source = m_Sources.at("imgui-viewport-depth");
    Handle<ImageResource> depth_source_image = depth_source.GetResource(0u);
    if (!source_image || !depth_source_image) return false;

    TUint32 width = source_image->GetImage()->GetWidth();
    TUint32 height = source_image->GetImage()->GetHeight();

    m_Width = width;
    m_Height = height;

    if (m_Renderpass == nullptr) {
      m_Renderpass = make_handle<Renderpass>();
      LoadStoreOp op;
      m_Renderpass->AddColorAttachment(source_image->GetImage()->GetFormat(), source_image->GetImage()->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      op.StoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      m_Renderpass->SetDepthAttachment(depth_source_image->GetImage()->GetFormat(), depth_source_image->GetImage()->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      m_Renderpass->Build(
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_NONE,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
      );
    }

    for (TUint32 i = 0; i < swapchain_image_count; i++) {
      Handle<ImageResource> this_source_image = source.GetResource(i);
      Handle<ImageResource> this_depth_source_image = depth_source.GetResource(i);

      Handle<Framebuffer> fbo = make_handle<Framebuffer>(this_source_image->GetImageView(), this_depth_source_image->GetImageView(), m_Renderpass, width, height);
      m_Framebuffers.push_back(fbo);
    }

    return true;
  }

  void LambertianPass::Execute(Handle<CommandBuffer> cmd, TUint32 frame_index) {
    MAU_GPU_ZONE(cmd->Get(), "LambertianPass::Execute");
    VkRect2D area = {
      .offset = { 0u, 0u },
      .extent = { m_Width, m_Height },
    };
    VkViewport viewport = {
      .x        = 0.0f,
      .y        = static_cast<float>(m_Height),
      .width    = static_cast<float>(m_Width),
      .height   = static_cast<float>(m_Height) * -1.0f,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
    };
    VkRect2D scissor = {
      .offset = { 0u, 0u },
      .extent = { m_Width, m_Height },
    };

    m_Renderpass->Begin(cmd, m_Framebuffers[frame_index], area);
    vkCmdSetViewport(cmd->Get(), 0u, 1u, &viewport);
    vkCmdSetScissor(cmd->Get(), 0u, 1u, &scissor);

    Renderer::Ref().Render(cmd, frame_index);

    m_Renderpass->End(cmd);
  }

}
