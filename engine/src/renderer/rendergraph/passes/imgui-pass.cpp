#include "imgui-pass.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include "graphics/vulkan-state.h"

namespace mau {

  ImGuiPass::ImGuiPass(): Pass("imgui-pass") {
    RegisterSource("$backbuffer");
  }

  ImGuiPass::~ImGuiPass() {
  }

  bool ImGuiPass::PostBuild(TUint32 swapchain_image_count) {
    m_Framebuffers.clear();

    const Source& source = m_Sources.at("$backbuffer");
    Handle<ImageResource> source_image = source.GetResource(0u);
    if (!source_image) return false;

    m_Width = source_image->GetImage()->GetWidth();
    m_Height = source_image->GetImage()->GetHeight();

    if (m_Renderpass == nullptr) {
      m_Renderpass = make_handle<Renderpass>();
      LoadStoreOp op;
      m_Renderpass->AddColorAttachment(source_image->GetImage()->GetFormat(), source_image->GetImage()->GetSamples(), op, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      m_Renderpass->Build(
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_NONE,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
      );
    }

    for (TUint32 i = 0; i < swapchain_image_count; i++) {
      Handle<ImageResource> this_source_image = source.GetResource(i);
      std::vector<Handle<ImageView>> image_view = { this_source_image->GetImageView() };
      Handle<Framebuffer> fbo = make_handle<Framebuffer>(image_view, m_Renderpass, m_Width, m_Height);
      m_Framebuffers.push_back(fbo);
    }

    return true;
  }

  void ImGuiPass::Execute(Handle<CommandBuffer> cmd, TUint32 frame_index) {
    MAU_GPU_ZONE(cmd->Get(), "ImGuiPass::Execute");
    VkRect2D area = {
      .offset = { 0u, 0u },
      .extent = { m_Width, m_Height },
    };

    m_Renderpass->Begin(cmd, m_Framebuffers[frame_index], area);
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->Get());
    m_Renderpass->End(cmd);
  }

}
