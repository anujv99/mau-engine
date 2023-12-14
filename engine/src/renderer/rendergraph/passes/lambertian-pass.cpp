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
    m_MSAAImages.clear();
    m_MSAAImageViews.clear();
    m_MSAADepthImages.clear();
    m_MSAADepthImageViews.clear();

    const Source& source = m_Sources.at("imgui-viewport-color");
    Handle<ImageResource> source_image = source.GetResource(0u);

    const Source& depth_source = m_Sources.at("imgui-viewport-depth");
    Handle<ImageResource> depth_source_image = depth_source.GetResource(0u);
    if (!source_image || !depth_source_image) return false;

    TUint32 width = source_image->GetImage()->GetWidth();
    TUint32 height = source_image->GetImage()->GetHeight();

    m_Width = width;
    m_Height = height;

    for (TUint32 i = 0; i < swapchain_image_count; i++) {
      Handle<Image> img = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_4_BIT, source_image->GetImage()->GetFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      Handle<ImageView> img_view = make_handle<ImageView>(img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
      Handle<Image> depth_img = make_handle<Image>(width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_SAMPLE_COUNT_4_BIT, depth_source_image->GetImage()->GetFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
      Handle<ImageView> depth_img_view = make_handle<ImageView>(depth_img, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

      m_MSAAImages.push_back(img);
      m_MSAAImageViews.push_back(img_view);
      m_MSAADepthImages.push_back(depth_img);
      m_MSAADepthImageViews.push_back(depth_img_view);
    }

    if (m_Renderpass == nullptr) {
      m_Renderpass = make_handle<Renderpass>();
      LoadStoreOp op;
      m_Renderpass->AddColorAttachment(m_MSAAImages[0]->GetFormat(), m_MSAAImages[0]->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      op.StoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      m_Renderpass->SetDepthAttachment(m_MSAADepthImages[0]->GetFormat(), m_MSAADepthImages[0]->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
      op.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      m_Renderpass->SetResolveAttachment(source_image->GetImage()->GetFormat(), source_image->GetImage()->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

      std::vector<Handle<ImageView>> image_views = { m_MSAAImageViews[i], m_MSAADepthImageViews[i], this_source_image->GetImageView() };

      Handle<Framebuffer> fbo = make_handle<Framebuffer>(image_views, m_Renderpass, width, height);
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
